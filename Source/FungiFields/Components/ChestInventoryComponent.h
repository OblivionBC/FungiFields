#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "ChestInventoryComponent.generated.h"

/**
 * Specialized inventory component for chests with replication enabled by default.
 * This ensures replication is set during construction via PostInitProperties.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UChestInventoryComponent : public UInventoryComponent
{
	GENERATED_BODY()

public:
	UChestInventoryComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void PostInitProperties() override;
};
