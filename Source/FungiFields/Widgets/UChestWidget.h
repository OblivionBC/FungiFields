#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UChestWidget.generated.h"

class UInventoryComponent;
class UInventorySlotWidget;
class UUniformGridPanel;
class UTextBlock;
struct FInventorySlot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChestWidgetClosed);

/**
 * Widget displaying both player inventory and chest inventory side-by-side.
 * Allows drag & drop between the two inventories.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UChestWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UChestWidget(const FObjectInitializer& ObjectInitializer);

	/** Setup the widget with player and chest inventories */
	UFUNCTION(BlueprintCallable, Category = "Chest Widget")
	void SetupInventories(UInventoryComponent* PlayerInventory, UInventoryComponent* ChestInventory);

	/** Close the widget */
	UFUNCTION(BlueprintCallable, Category = "Chest Widget")
	void CloseWidget();

	/** Delegate broadcast when chest widget is closed */
	UPROPERTY(BlueprintAssignable, Category = "Chest Widget")
	FOnChestWidgetClosed OnChestWidgetClosed;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void OnCloseButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> PlayerInventoryGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> ChestInventoryGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerInventoryLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ChestInventoryLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;

	/** Number of columns in the inventory grids */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest Widget")
	int32 GridColumns = 9;

	/** Total number of player inventory slots */
	static constexpr int32 PlayerSlotCount = 27;

	/** Total number of chest inventory slots */
	static constexpr int32 ChestSlotCount = 18;

private:
	UFUNCTION()
	void OnPlayerInventoryChanged();

	UFUNCTION()
	void OnChestInventoryChanged();

	void UpdatePlayerSlots();
	void UpdateChestSlots();
	UInventorySlotWidget* GetOrCreatePlayerSlotWidget(int32 SlotIndex);
	UInventorySlotWidget* GetOrCreateChestSlotWidget(int32 SlotIndex);
	
	UFUNCTION()
	void HandlePlayerSlotClicked(int32 SlotIndex, int32 InventorySourceID);
	
	UFUNCTION()
	void HandleChestSlotClicked(int32 SlotIndex, int32 InventorySourceID);
	
	UFUNCTION()
	void HandlePlayerDragStarted(int32 SlotIndex, int32 InventorySourceID, const FInventorySlot& SlotData);
	
	UFUNCTION()
	void HandleChestDragStarted(int32 SlotIndex, int32 InventorySourceID, const FInventorySlot& SlotData);
	
	UFUNCTION()
	void HandleSlotDropped(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID);
	
	/** Handle drag & drop between inventories */
	bool HandleItemTransfer(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID);

	UPROPERTY()
	TObjectPtr<UInventoryComponent> PlayerInventory;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> ChestInventory;

	UPROPERTY()
	TArray<TObjectPtr<UInventorySlotWidget>> PlayerSlotWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UInventorySlotWidget>> ChestSlotWidgets;

	/** Slot widget class to use */
	UPROPERTY(EditDefaultsOnly, Category = "Chest Widget")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;
};



