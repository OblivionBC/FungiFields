#include "FarmerAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "FarmerBlackboardKeys.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "../Components/UFarmingComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Components/AI/FarmerTargetingComponent.h"
#include "../Components/UVillagerNeedsComponent.h"
#include "../Interfaces/IHarvestableInterface.h"
#include "../Interfaces/IFarmableInterface.h"
#include "../Data/UToolDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "../ENUM/EFarmerRole.h"

DEFINE_LOG_CATEGORY_STATIC(LogAIFarmer, Log, All);

AFarmerAIController::AFarmerAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFarmerAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CachedVillager = Cast<AFarmerVillagerCharacter>(InPawn);
	FarmingComponent = CachedVillager ? CachedVillager->GetFarmingComponent() : nullptr;
	TargetingComponent = CachedVillager ? CachedVillager->GetTargetingComponent() : nullptr;
	CurrentTarget = nullptr;
	CachedToolData = nullptr;
	CachedSeedData = nullptr;
	CachedSeedSlotIndex = INDEX_NONE;
	JobState = EFarmerJobState::Idle;

	if (!FarmingComponent || !TargetingComponent)
	{
		LogDebug(TEXT("Missing required farming/targeting components."), ELogVerbosity::Warning);
		return;
	}

	if (bUseBehaviorTree && FarmingBehaviorTree)
	{
		// BT mode: run the behavior tree; services and tasks drive all logic from here on.
		// Initialize the blackboard with static villager data before the first BT tick.
		RunBehaviorTree(FarmingBehaviorTree);

		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			if (CachedVillager)
				BB->SetValueAsVector(FarmerBBKeys::HomeLocation, CachedVillager->GetHomeLocation());
		}
	}
	else
	{
		// Legacy timer-brain — runs until bUseBehaviorTree is enabled.
		if (UWorld* World = GetWorld())
			World->GetTimerManager().SetTimer(BrainTimerHandle, this, &AFarmerAIController::TickBrain, BrainTickInterval, true);
	}
}

void AFarmerAIController::OnUnPossess()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BrainTimerHandle);
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		World->GetTimerManager().ClearTimer(WanderCooldownTimer);
	}

	ClearCurrentTarget();
	FarmingComponent = nullptr;
	TargetingComponent = nullptr;
	CachedVillager = nullptr;
	CachedToolData = nullptr;
	CachedSeedData = nullptr;
	CachedSeedSlotIndex = INDEX_NONE;
	JobState = EFarmerJobState::Idle;

	Super::OnUnPossess();
}

void AFarmerAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (JobState == EFarmerJobState::Wandering)
	{
		// Transition to Idle so TickBrain can re-evaluate available work before the next wander
		JobState = EFarmerJobState::Idle;

		if (CachedVillager)
		{
			const float Delay = FMath::RandRange(CachedVillager->WanderCooldownMin, CachedVillager->WanderCooldownMax);
			GetWorldTimerManager().SetTimer(WanderCooldownTimer, this, &AFarmerAIController::BeginWander, Delay, false);
		}
		return;
	}

	if (JobState != EFarmerJobState::MovingToTarget)
	{
		return;
	}

	if (Result.IsSuccess())
	{
		LogDebug(TEXT("Move reached target range."));
		ProcessCurrentTarget();
		return;
	}

	LogDebug(FString::Printf(TEXT("Move failed with result code %d."), static_cast<int32>(Result.Code)), ELogVerbosity::Warning);
	ClearCurrentTarget();
	BeginCooldown();
}

void AFarmerAIController::TickBrain()
{
	if (!FarmingComponent || !TargetingComponent ||
		JobState == EFarmerJobState::Cooldown ||
		JobState == EFarmerJobState::Wandering)
	{
		return;
	}

	if (CachedVillager)
	{
		if (UVillagerNeedsComponent* Needs = CachedVillager->GetNeedsComponent())
		{
			const float WorkSpeed = Needs->GetWorkSpeedMultiplier();
			if (WorkSpeed <= 0.f)
			{
				LogDebug(TEXT("Villager is starving — skipping work tick."), ELogVerbosity::Verbose);
				ClearCurrentTarget();
				BeginWander();
				return;
			}
			if (WorkSpeed < 1.f && FMath::FRand() > WorkSpeed)
			{
				return;
			}
		}

		LogDebug(FString::Printf(TEXT("TickBrain: State=%d Role=%s Tool=%s Target=%s"),
			static_cast<int32>(JobState),
			*UEnum::GetDisplayValueAsText(CachedVillager->GetAssignedRole()).ToString(),
			*GetNameSafe(CachedToolData.Get()),
			*GetNameSafe(CurrentTarget.Get())),
			ELogVerbosity::VeryVerbose);
	}

	if (!IsValid(CurrentTarget))
	{
		AcquireTargetForRole();
	}

	if (!IsValid(CurrentTarget))
	{
		BeginWander();
		return;
	}

	ProcessCurrentTarget();
}

void AFarmerAIController::AcquireTargetForRole()
{
	if (!CachedVillager)
		return;

	GetWorldTimerManager().ClearTimer(WanderCooldownTimer);

	UToolDataAsset* FoundTool = CachedVillager->FindEquippedToolForRole();
	if (!FoundTool)
	{
		LogDebug(TEXT("No matching tool found in inventory for current role. Idling."), ELogVerbosity::Warning);
		JobState = EFarmerJobState::Idle;
		return;
	}

	CachedToolData = FoundTool;
	FarmingComponent->SetEquippedTool(FoundTool->ToolType, FoundTool->ToolPower);
	CachedSeedData = nullptr;
	CachedSeedSlotIndex = INDEX_NONE;

	switch (CachedVillager->GetAssignedRole())
	{
	case EFarmerRole::Harvester:
		AcquireHarvestTarget();
		break;
	case EFarmerRole::Planter:
		AcquirePlantTarget();
		break;
	case EFarmerRole::Waterer:
		AcquireWaterTarget();
		break;
	default:
		LogDebug(TEXT("Unknown role. Idling."), ELogVerbosity::Verbose);
		JobState = EFarmerJobState::Idle;
		break;
	}
}

void AFarmerAIController::AcquireHarvestTarget()
{
	JobState = EFarmerJobState::AcquiringTarget;

	if (!CachedVillager)
		return;

	CurrentTarget = TargetingComponent->FindBestHarvestTarget(CachedVillager->GetActorLocation(), FarmingComponent, GetActiveToolType(), CachedVillager->GetAssignedPlotsRaw());
	if (IsValid(CurrentTarget))
	{
		LogDebug(FString::Printf(TEXT("Harvest target acquired: %s"), *GetNameSafe(CurrentTarget)));
	}
}

void AFarmerAIController::AcquirePlantTarget()
{
	JobState = EFarmerJobState::AcquiringTarget;

	if (!CachedVillager)
		return;

	int32 SeedSlotIndex = INDEX_NONE;
	USeedDataAsset* FoundSeed = CachedVillager->FindSeedInInventory(SeedSlotIndex);
	if (!FoundSeed)
	{
		LogDebug(TEXT("No seeds found in inventory for planter role. Idling."), ELogVerbosity::Warning);
		JobState = EFarmerJobState::Idle;
		return;
	}

	CachedSeedData = FoundSeed;
	CachedSeedSlotIndex = SeedSlotIndex;
	FarmingComponent->SetEquippedSeedData(FoundSeed);

	CurrentTarget = TargetingComponent->FindBestPlantTarget(CachedVillager->GetActorLocation(), FoundSeed,
		CachedVillager->GetAssignedPlotsRaw());
	if (IsValid(CurrentTarget))
	{
		LogDebug(FString::Printf(TEXT("Plant target acquired: %s"), *GetNameSafe(CurrentTarget)));
	}
}

void AFarmerAIController::AcquireWaterTarget()
{
	JobState = EFarmerJobState::AcquiringTarget;

	if (!CachedVillager)
		return;

	CurrentTarget = TargetingComponent->FindBestWaterTarget(CachedVillager->GetActorLocation(),
		CachedVillager->GetAssignedPlotsRaw());
	if (IsValid(CurrentTarget))
	{
		LogDebug(FString::Printf(TEXT("Water target acquired: %s"), *GetNameSafe(CurrentTarget)));
	}
}

void AFarmerAIController::ProcessCurrentTarget()
{
	if (!IsValid(CurrentTarget) || !FarmingComponent)
	{
		ClearCurrentTarget();
		BeginCooldown();
		return;
	}

	const EToolType ActiveToolType = GetActiveToolType();
	if (!FarmingComponent->CanPerformFarmingAction(CurrentTarget, ActiveToolType, CachedSeedData.Get()))
	{
		LogDebug(TEXT("Action validation failed via CanPerformFarmingAction."), ELogVerbosity::Verbose);
		ClearCurrentTarget();
		BeginCooldown();
		return;
	}

	FVector ActionLocation = FVector::ZeroVector;
	float InteractionRange = FallbackInteractionRange;
	if (!TryGetTargetActionData(ActionLocation, InteractionRange))
	{
		LogDebug(TEXT("Target missing action data interface."), ELogVerbosity::Warning);
		ClearCurrentTarget();
		BeginCooldown();
		return;
	}

	if (!IsTargetInRange(ActionLocation, InteractionRange))
	{
		const float AcceptanceRadius = FMath::Max(1.0f, InteractionRange + MoveAcceptanceBuffer);
		const EPathFollowingRequestResult::Type MoveResult = MoveToLocation(ActionLocation, AcceptanceRadius, true, true, false, true, nullptr, true);
		JobState = EFarmerJobState::MovingToTarget;
		LogDebug(FString::Printf(TEXT("Move started toward %s (acceptance %.1f)."), *GetNameSafe(CurrentTarget), AcceptanceRadius));

		if (MoveResult == EPathFollowingRequestResult::Failed)
		{
			LogDebug(TEXT("MoveToLocation failed immediately."), ELogVerbosity::Warning);
			ClearCurrentTarget();
			BeginCooldown();
		}
		return;
	}

	JobState = EFarmerJobState::PerformingAction;

	const float ToolPower = GetActiveToolPower();
	const bool bActionSuccess = FarmingComponent->ExecuteFarmingAction(CurrentTarget, ActionLocation, ActiveToolType, ToolPower, CachedSeedData.Get());

	if (bActionSuccess && IsValid(CachedSeedData) && CachedSeedSlotIndex != INDEX_NONE)
	{
		// ExecuteFarmingAction uses EquippedSlotIndexCached (INDEX_NONE for AI), so we consume manually
		if (UInventoryComponent* InvComp = CachedVillager ? CachedVillager->GetInventoryComponent() : nullptr)
		{
			InvComp->ConsumeFromSlot(CachedSeedSlotIndex, 1);
		}
		CachedSeedData = nullptr;
		CachedSeedSlotIndex = INDEX_NONE;
	}

	LogDebug(
		FString::Printf(TEXT("Action %s on %s."), bActionSuccess ? TEXT("succeeded") : TEXT("failed"), *GetNameSafe(CurrentTarget)),
		bActionSuccess ? ELogVerbosity::Log : ELogVerbosity::Warning
	);

	ClearCurrentTarget();
	BeginCooldown();
}

void AFarmerAIController::BeginWander()
{
	if (!CachedVillager)
		return;

	JobState = EFarmerJobState::Wandering;

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSystem)
	{
		GetWorldTimerManager().SetTimer(WanderCooldownTimer, this, &AFarmerAIController::BeginWander, CachedVillager->WanderCooldownMin, false);
		return;
	}

	FNavLocation RandomLocation;
	const bool bFound = NavSystem->GetRandomReachablePointInRadius(CachedVillager->GetHomeLocation(), CachedVillager->WanderRadius, RandomLocation);
	if (!bFound)
	{
		GetWorldTimerManager().SetTimer(WanderCooldownTimer, this, &AFarmerAIController::BeginWander, CachedVillager->WanderCooldownMin, false);
		return;
	}

	const EPathFollowingRequestResult::Type MoveResult = MoveToLocation(RandomLocation.Location, 50.0f, true);
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		JobState = EFarmerJobState::Idle;
		GetWorldTimerManager().SetTimer(WanderCooldownTimer, this, &AFarmerAIController::BeginWander, CachedVillager->WanderCooldownMin, false);
	}
}

void AFarmerAIController::ResetToIdle()
{
	if (bUseBehaviorTree)
	{
		// In BT mode, clear the target key so the tree re-acquires on the next tick.
		if (UBlackboardComponent* BB = GetBlackboardComponent())
			BB->ClearValue(FarmerBBKeys::TargetActor);
		return;
	}

	// Legacy timer-brain path.
	ClearCurrentTarget();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		World->GetTimerManager().ClearTimer(WanderCooldownTimer);
	}

	CachedToolData = nullptr;
	CachedSeedData = nullptr;
	CachedSeedSlotIndex = INDEX_NONE;
	JobState = EFarmerJobState::Idle;
}

void AFarmerAIController::BeginCooldown()
{
	JobState = EFarmerJobState::Cooldown;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CooldownTimerHandle, this, &AFarmerAIController::HandleCooldownFinished, ActionCooldownSeconds, false);
	}
}

void AFarmerAIController::HandleCooldownFinished()
{
	JobState = EFarmerJobState::Idle;
}

void AFarmerAIController::ClearCurrentTarget()
{
	StopMovement();
	CurrentTarget = nullptr;
}

bool AFarmerAIController::TryGetTargetActionData(FVector& OutActionLocation, float& OutInteractionRange) const
{
	if (!IsValid(CurrentTarget))
	{
		return false;
	}

	if (CurrentTarget->Implements<UFarmableInterface>())
	{
		OutActionLocation = IFarmableInterface::Execute_GetActionLocation(CurrentTarget);
		OutInteractionRange = FMath::Max(1.0f, IFarmableInterface::Execute_GetInteractionRange(CurrentTarget));
		return true;
	}

	if (CurrentTarget->Implements<UHarvestableInterface>())
	{
		OutActionLocation = IHarvestableInterface::Execute_GetActionLocation(CurrentTarget);
		OutInteractionRange = FMath::Max(1.0f, IHarvestableInterface::Execute_GetInteractionRange(CurrentTarget));
		return true;
	}

	return false;
}

bool AFarmerAIController::IsTargetInRange(const FVector& ActionLocation, float InteractionRange) const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	const float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), ActionLocation);
	return Distance <= InteractionRange;
}

EToolType AFarmerAIController::GetActiveToolType() const
{
	return CachedToolData ? CachedToolData->ToolType : EToolType::None;
}

float AFarmerAIController::GetActiveToolPower() const
{
	return CachedToolData ? CachedToolData->ToolPower : 1.0f;
}

bool AFarmerAIController::IsHarvesterRoleActive() const
{
	return CachedVillager && CachedVillager->GetAssignedRole() == EFarmerRole::Harvester;
}

bool AFarmerAIController::IsDebugEnabled() const
{
	return CachedVillager ? CachedVillager->IsAIDebugLoggingEnabled() : true;
}

void AFarmerAIController::LogDebug(const FString& Message, ELogVerbosity::Type Verbosity) const
{
	if (!IsDebugEnabled())
	{
		return;
	}

	const FString Formatted = FString::Printf(TEXT("[%s] %s"), *GetNameSafe(GetPawn()), *Message);
	switch (Verbosity)
	{
	case ELogVerbosity::Fatal:
		UE_LOG(LogAIFarmer, Fatal, TEXT("%s"), *Formatted);
		break;
	case ELogVerbosity::Error:
		UE_LOG(LogAIFarmer, Error, TEXT("%s"), *Formatted);
		break;
	case ELogVerbosity::Warning:
		UE_LOG(LogAIFarmer, Warning, TEXT("%s"), *Formatted);
		break;
	case ELogVerbosity::Display:
		UE_LOG(LogAIFarmer, Display, TEXT("%s"), *Formatted);
		break;
	case ELogVerbosity::Verbose:
		UE_LOG(LogAIFarmer, Verbose, TEXT("%s"), *Formatted);
		break;
	case ELogVerbosity::VeryVerbose:
		UE_LOG(LogAIFarmer, VeryVerbose, TEXT("%s"), *Formatted);
		break;
	default:
		UE_LOG(LogAIFarmer, Log, TEXT("%s"), *Formatted);
		break;
	}
}
