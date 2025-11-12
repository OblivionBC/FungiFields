#pragma once

#include "CoreMinimal.h"
#include "FInventorySlot.generated.h"

class UItemDataAsset;

/**
 * Struct representing a single inventory slot.
 * Contains a reference to the item definition and the count of items in the slot.
 * USTRUCT is appropriate here as slots are lightweight data containers, not garbage collected.
 */
USTRUCT(BlueprintType)
struct FUNGIFIELDS_API FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Slot")
	TObjectPtr<const UItemDataAsset> ItemDefinition = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Slot")
	int32 Count = 0;

	bool IsEmpty() const { return Count <= 0 || ItemDefinition == nullptr; }
};

