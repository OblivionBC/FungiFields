#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateVillagerNeeds.generated.h"

/**
 * BT service that mirrors UVillagerNeedsComponent state into the Blackboard each tick.
 *
 * Updates: bIsStarving, bIsHungry, WorkSpeed, HomeLocation.
 *
 * Attach to the Root node so it always runs regardless of which branch is active.
 * Default interval matches the NeedsComponent tick rate (~1 game-hour resolution is fine).
 */
UCLASS()
class FUNGIFIELDS_API UBTService_UpdateVillagerNeeds : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateVillagerNeeds();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
