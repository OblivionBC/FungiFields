#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_VillagerWander.generated.h"

/**
 * Picks a random reachable nav point within the villager's WanderRadius around their
 * HomeLocation and writes it to the WanderTarget Blackboard key.
 *
 * This task is synchronous — it only sets the destination.
 * Pair it with a built-in "Move To" task (set Key = WanderTarget) in the BT graph.
 *
 * Succeeds when a valid nav point was found.
 * Fails when the nav system returns no reachable point (rare).
 */
UCLASS()
class FUNGIFIELDS_API UBTTask_VillagerWander : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_VillagerWander();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
