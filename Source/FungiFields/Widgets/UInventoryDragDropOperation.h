#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "../Inventory/FInventorySlot.h"
#include "UInventoryDragDropOperation.generated.h"

/**
 * Custom drag drop operation for inventory items.
 * Stores information about the source slot and inventory.
 */
UCLASS()
class FUNGIFIELDS_API UInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** Source slot index */
	UPROPERTY(BlueprintReadOnly)
	int32 SourceSlotIndex = INDEX_NONE;

	/** Source inventory ID (0 = Player, 1 = Chest, etc.) */
	UPROPERTY(BlueprintReadOnly)
	int32 SourceInventoryID = 0;

	/** Slot data being dragged */
	UPROPERTY(BlueprintReadOnly)
	FInventorySlot SlotData;
};

