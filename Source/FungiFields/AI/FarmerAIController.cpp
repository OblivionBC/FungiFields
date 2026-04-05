#include "FarmerAIController.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "../Components/UFarmingComponent.h"
#include "../Components/AI/FarmerTargetingComponent.h"
#include "../Interfaces/IHarvestableInterface.h"
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
	JobState = EFarmerJobState::Idle;

	if (!FarmingComponent || !TargetingComponent)
	{
		LogDebug(TEXT("Missing required farming/targeting components."), ELogVerbosity::Warning);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(BrainTimerHandle, this, &AFarmerAIController::TickBrain, BrainTickInterval, true);
	}
}

void AFarmerAIController::OnUnPossess()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BrainTimerHandle);
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}

	ClearCurrentTarget();
	FarmingComponent = nullptr;
	TargetingComponent = nullptr;
	CachedVillager = nullptr;
	JobState = EFarmerJobState::Idle;

	Super::OnUnPossess();
}

void AFarmerAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

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
	if (!FarmingComponent || !TargetingComponent || JobState == EFarmerJobState::Cooldown)
	{
		return;
	}

	if (!IsValid(CurrentTarget))
	{
		AcquireTargetForRole();
	}

	if (!IsValid(CurrentTarget))
	{
		JobState = EFarmerJobState::Idle;
		return;
	}

	ProcessCurrentTarget();
}

void AFarmerAIController::AcquireHarvestTarget()
{
	JobState = EFarmerJobState::AcquiringTarget;

	if (!CachedVillager)
		return;

	CurrentTarget = TargetingComponent->FindBestHarvestTarget(CachedVillager->GetActorLocation(), FarmingComponent, GetActiveToolType());
	if (IsValid(CurrentTarget))
	{
		LogDebug(FString::Printf(TEXT("Target acquired: %s"), *GetNameSafe(CurrentTarget)));
	}
}

void AFarmerAIController::AcquireTargetForRole()
{
	if (!IsHarvesterRoleActive())
	{
		LogDebug(TEXT("Current role is not implemented yet. Idling."), ELogVerbosity::Verbose);
		return;
	}

	AcquireHarvestTarget();
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
	if (!FarmingComponent->CanPerformFarmingAction(CurrentTarget, ActiveToolType))
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
		LogDebug(TEXT("Target missing IHarvestableInterface action data."), ELogVerbosity::Warning);
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
	const bool bActionSuccess = FarmingComponent->ExecuteFarmingAction(CurrentTarget, ActionLocation, ActiveToolType, ToolPower, nullptr);

	LogDebug(FString::Printf(TEXT("Harvest action %s on %s."), bActionSuccess ? TEXT("succeeded") : TEXT("failed"), *GetNameSafe(CurrentTarget)), bActionSuccess ? ELogVerbosity::Log : ELogVerbosity::Warning);

	ClearCurrentTarget();
	BeginCooldown();
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
	if (!IsValid(CurrentTarget) || !CurrentTarget->Implements<UHarvestableInterface>())
	{
		return false;
	}

	OutActionLocation = IHarvestableInterface::Execute_GetActionLocation(CurrentTarget);
	OutInteractionRange = FMath::Max(1.0f, IHarvestableInterface::Execute_GetInteractionRange(CurrentTarget));
	return true;
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
	return CachedVillager ? CachedVillager->GetRoleToolType() : EToolType::Scythe;
}

float AFarmerAIController::GetActiveToolPower() const
{
	return CachedVillager ? CachedVillager->GetRoleToolPower() : 1.0f;
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
