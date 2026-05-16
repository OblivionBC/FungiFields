#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PerformFarmAction.generated.h"

/**
 * Executes the farming action on the actor stored in TargetActor.
 * Assumes the villager is already in range (BT graph should use Move To before this task).
 *
 * Reading:  TargetActor (FarmerBBKeys::TargetActor).
 * Clears:   TargetActor on completion (success or failure) so the next BT tick re-acquires.
 *
 * Succeeds if the action was executed successfully.
 * Fails if the target is invalid, out of range, or the action fails.
 */
UCLASS()
class FUNGIFIELDS_API UBTTask_PerformFarmAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PerformFarmAction();

	/** Extra buffer added to the target's reported interaction range when checking distance. */
	UPROPERTY(EditAnywhere, Category = "Farming", meta = (ClampMin = "0.0"))
	float RangeBuffer = 15.f;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
