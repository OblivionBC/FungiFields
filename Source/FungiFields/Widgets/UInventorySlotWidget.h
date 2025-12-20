#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Inventory/FInventorySlot.h"
#include "UInventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;
class UItemDataAsset;

/**
 * Reusable inventory slot widget with drag & drop support.
 * Can be used in hotbar, backpack, and chest inventories.
 */
UCLASS()
class FUNGIFIELDS_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInventorySlotWidget(const FObjectInitializer& ObjectInitializer);

	/** Initialize the slot with data */
	UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
	void SetSlotData(const FInventorySlot& SlotData, int32 SlotIndex, bool bIsEquipped = false);

	/** Get the slot index this widget represents */
	UFUNCTION(BlueprintPure, Category = "Inventory Slot")
	int32 GetSlotIndex() const { return SlotIndex; }

	/** Get the current slot data */
	UFUNCTION(BlueprintPure, Category = "Inventory Slot")
	const FInventorySlot& GetSlotData() const { return CurrentSlotData; }

	/** Set whether this slot is equipped (for visual highlighting) */
	UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
	void SetEquipped(bool bIsEquipped);

	/** Set the inventory source identifier (for drag & drop) */
	UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
	void SetInventorySource(int32 SourceID) { InventorySourceID = SourceID; }

	/** Get the inventory source identifier */
	UFUNCTION(BlueprintPure, Category = "Inventory Slot")
	int32 GetInventorySource() const { return InventorySourceID; }

	/** Delegate for when slot is clicked */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlotClicked, int32, SlotIndex, int32, InventorySourceID);
	UPROPERTY(BlueprintAssignable, Category = "Inventory Slot")
	FOnSlotClicked OnSlotClicked;

	/** Delegate for when drag starts */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDragStarted, int32, SlotIndex, int32, InventorySourceID, const FInventorySlot&, SlotData);
	UPROPERTY(BlueprintAssignable, Category = "Inventory Slot")
	FOnDragStarted OnDragStarted;

	/** Delegate for when item is dropped on this slot */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSlotDropped, int32, SourceSlotIndex, int32, SourceInventoryID, int32, TargetSlotIndex, int32, TargetInventoryID);
	UPROPERTY(BlueprintAssignable, Category = "Inventory Slot")
	FOnSlotDropped OnSlotDropped;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& MyGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	/** Update visual representation of the slot */
	void UpdateSlotVisuals();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SlotBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemCount;

private:
	FInventorySlot CurrentSlotData;
	int32 SlotIndex = INDEX_NONE;
	int32 InventorySourceID = 0; // 0 = Player, 1 = Chest, etc.
	bool bIsEquipped = false;
	bool bIsDragTarget = false;
};

