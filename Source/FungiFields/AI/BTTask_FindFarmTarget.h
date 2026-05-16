#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindFarmTarget.generated.h"

/**
 * Picks the best available farming target for the villager's current role and writes
 * it to the TargetActor Blackboard key.
 *
 * Succeeds when a valid target is found.
 * Fails when no target is available (BT falls through to the next branch, e.g. wander).
 *
 * Reading: VillagerRole, AssignedPlots — from the controlled pawn directly.
 * Writing: TargetActor (FarmerBBKeys::TargetActor).
 */
UCLASS()
class FUNGIFIELDS_API UBTTask_FindFarmTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindFarmTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
