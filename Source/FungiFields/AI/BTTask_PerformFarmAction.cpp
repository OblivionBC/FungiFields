#include "BTTask_PerformFarmAction.h"
#include "FarmerBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "../Components/UFarmingComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Interfaces/IFarmableInterface.h"
#include "../Interfaces/IHarvestableInterface.h"
#include "../Data/USeedDataAsset.h"
#include "../ENUM/EFarmerRole.h"

UBTTask_PerformFarmAction::UBTTask_PerformFarmAction()
{
	NodeName = TEXT("Perform Farm Action");
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_PerformFarmAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController*        AIC = OwnerComp.GetAIOwner();
	AFarmerVillagerCharacter* Villager = AIC ? Cast<AFarmerVillagerCharacter>(AIC->GetPawn()) : nullptr;

	if (!BB || !Villager) return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(FarmerBBKeys::TargetActor));
	if (!IsValid(Target))
	{
		BB->ClearValue(FarmerBBKeys::TargetActor);
		return EBTNodeResult::Failed;
	}

	UFarmingComponent* Farming = Villager->GetFarmingComponent();
	if (!Farming)
	{
		BB->ClearValue(FarmerBBKeys::TargetActor);
		return EBTNodeResult::Failed;
	}

	// Resolve the seed for planters (FarmingComponent must already have it set by BTTask_FindFarmTarget).
	USeedDataAsset* SeedData = nullptr;
	int32 SeedSlotIndex = INDEX_NONE;
	if (Villager->GetAssignedRole() == EFarmerRole::Planter)
	{
		SeedData = Villager->FindSeedInInventory(SeedSlotIndex);
		if (!SeedData)
		{
			BB->ClearValue(FarmerBBKeys::TargetActor);
			return EBTNodeResult::Failed;
		}
	}

	// Action-location / range check.
	FVector ActionLocation = Target->GetActorLocation();
	float InteractionRange = 100.f;

	if (Target->Implements<UFarmableInterface>())
	{
		ActionLocation   = IFarmableInterface::Execute_GetActionLocation(Target);
		InteractionRange = IFarmableInterface::Execute_GetInteractionRange(Target);
	}
	else if (Target->Implements<UHarvestableInterface>())
	{
		ActionLocation   = IHarvestableInterface::Execute_GetActionLocation(Target);
		InteractionRange = IHarvestableInterface::Execute_GetInteractionRange(Target);
	}

	const float Dist = FVector::Dist(Villager->GetActorLocation(), ActionLocation);
	if (Dist > InteractionRange + RangeBuffer)
	{
		// Still out of range after Move To — let the BT retry on the next tick.
		BB->ClearValue(FarmerBBKeys::TargetActor);
		return EBTNodeResult::Failed;
	}

	const EToolType ToolType = Farming->GetCurrentToolType();
	const float     ToolPower = Farming->GetCurrentToolPower();

	const bool bSuccess = Farming->ExecuteFarmingAction(Target, ActionLocation, ToolType, ToolPower, SeedData);

	if (bSuccess && SeedData && SeedSlotIndex != INDEX_NONE)
	{
		if (UInventoryComponent* InvComp = Villager->GetInventoryComponent())
			InvComp->ConsumeFromSlot(SeedSlotIndex, 1);
	}

	BB->ClearValue(FarmerBBKeys::TargetActor);
	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_PerformFarmAction::GetStaticDescription() const
{
	return TEXT("Executes the farming action on TargetActor. Place after a Move To node in the BT graph.\nClears TargetActor on completion.");
}
