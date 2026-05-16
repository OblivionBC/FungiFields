#include "BTTask_FindFarmTarget.h"
#include "FarmerBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "../Components/AI/FarmerTargetingComponent.h"
#include "../Components/UFarmingComponent.h"
#include "../Components/InventoryComponent.h"
#include "../ENUM/EFarmerRole.h"
#include "../Data/UToolDataAsset.h"
#include "../Data/USeedDataAsset.h"

UBTTask_FindFarmTarget::UBTTask_FindFarmTarget()
{
	NodeName = TEXT("Find Farm Target");
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_FindFarmTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	AFarmerVillagerCharacter* Villager = AIC ? Cast<AFarmerVillagerCharacter>(AIC->GetPawn()) : nullptr;
	if (!Villager) return EBTNodeResult::Failed;

	UFarmerTargetingComponent* Targeting = Villager->GetTargetingComponent();
	UFarmingComponent*         Farming   = Villager->GetFarmingComponent();
	if (!Targeting || !Farming) return EBTNodeResult::Failed;

	UToolDataAsset* Tool = Villager->FindEquippedToolForRole();
	if (!Tool) return EBTNodeResult::Failed;

	Farming->SetEquippedTool(Tool->ToolType, Tool->ToolPower);

	AActor* Target = nullptr;
	const EFarmerRole Role = Villager->GetAssignedRole();

	switch (Role)
	{
	case EFarmerRole::Harvester:
		Target = Targeting->FindBestHarvestTarget(Villager->GetActorLocation(), Farming, Tool->ToolType, Villager->GetAssignedPlotsRaw());
		break;

	case EFarmerRole::Planter:
	{
		int32 SeedSlot = INDEX_NONE;
		USeedDataAsset* Seed = Villager->FindSeedInInventory(SeedSlot);
		if (!Seed) return EBTNodeResult::Failed;
		Farming->SetEquippedSeedData(Seed);
		Target = Targeting->FindBestPlantTarget(Villager->GetActorLocation(), Seed, Villager->GetAssignedPlotsRaw());
		break;
	}

	case EFarmerRole::Waterer:
		Target = Targeting->FindBestWaterTarget(Villager->GetActorLocation(), Villager->GetAssignedPlotsRaw());
		break;

	default:
		return EBTNodeResult::Failed;
	}

	if (!IsValid(Target)) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB) BB->SetValueAsObject(FarmerBBKeys::TargetActor, Target);

	return EBTNodeResult::Succeeded;
}

FString UBTTask_FindFarmTarget::GetStaticDescription() const
{
	return TEXT("Finds the best harvest/plant/water target for the villager's current role.\nWrites result to TargetActor.");
}
