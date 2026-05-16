#include "BTTask_VillagerWander.h"
#include "FarmerBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "../Actors/ASoilPlot.h"

UBTTask_VillagerWander::UBTTask_VillagerWander()
{
	NodeName = TEXT("Villager Wander");
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_VillagerWander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	AFarmerVillagerCharacter* Villager = AIC ? Cast<AFarmerVillagerCharacter>(AIC->GetPawn()) : nullptr;
	if (!Villager) return EBTNodeResult::Failed;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(AIC->GetWorld());
	if (!NavSys) return EBTNodeResult::Failed;

	FVector Origin = Villager->GetHomeLocation();
	TArray<TWeakObjectPtr<ASoilPlot>>& Plots = Villager->GetAssignedPlotsRaw();
	FVector PlotSum = FVector::ZeroVector;
	int32 ValidCount = 0;
	for (const TWeakObjectPtr<ASoilPlot>& PlotPtr : Plots)
	{
		if (PlotPtr.IsValid())
		{
			PlotSum += PlotPtr->GetActorLocation();
			++ValidCount;
		}
	}
	if (ValidCount > 0)
	{
		Origin = PlotSum / static_cast<float>(ValidCount);
	}

	FNavLocation RandomPoint;
	const bool bFound = NavSys->GetRandomReachablePointInRadius(
		Origin, Villager->WanderRadius, RandomPoint);

	if (!bFound) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB) BB->SetValueAsVector(FarmerBBKeys::WanderTarget, RandomPoint.Location);

	return EBTNodeResult::Succeeded;
}

FString UBTTask_VillagerWander::GetStaticDescription() const
{
	return TEXT("Picks a random nav point within WanderRadius around the centroid of assigned plots (or HomeLocation if none).\nWrites result to WanderTarget. Pair with a Move To node.");
}
