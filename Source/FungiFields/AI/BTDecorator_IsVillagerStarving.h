#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsVillagerStarving.generated.h"

/**
 * Condition decorator: passes when the villager's bIsStarving Blackboard key is true.
 *
 * Usage: Attach to the "starving wander" branch so it only executes when the villager
 * cannot work. Enable "Observer Aborts = Self" so the wander stops the moment the
 * player feeds the villager and bIsStarving flips to false.
 */
UCLASS()
class FUNGIFIELDS_API UBTDecorator_IsVillagerStarving : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsVillagerStarving();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
};
