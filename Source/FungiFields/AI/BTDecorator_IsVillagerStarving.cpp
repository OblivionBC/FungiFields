#include "BTDecorator_IsVillagerStarving.h"
#include "FarmerBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsVillagerStarving::UBTDecorator_IsVillagerStarving()
{
	NodeName = TEXT("Is Villager Starving");
	// Default: abort self when condition becomes false (player feeds the villager mid-wander).
	FlowAbortMode = EBTFlowAbortMode::Self;
	bNotifyTick = false;
}

bool UBTDecorator_IsVillagerStarving::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	return BB && BB->GetValueAsBool(FarmerBBKeys::bIsStarving);
}

FString UBTDecorator_IsVillagerStarving::GetStaticDescription() const
{
	return TEXT("Passes when bIsStarving == true.\nSet Observer Aborts = Self to stop wander when fed.");
}
