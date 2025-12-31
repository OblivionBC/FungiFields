#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UBackpackWidget.generated.h"

class UInventoryComponent;
class AFungiFieldsCharacter;
class UInventorySlotWidget;
class UUniformGridPanel;
struct FInventorySlot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackpackClosed);

/**
 * Widget displaying the full player inventory (27 slots including hotbar).
 * Accessible via Tab key input.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UBackpackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBackpackWidget(const FObjectInitializer& ObjectInitializer);

	/** Close the backpack widget */
	UFUNCTION(BlueprintCallable, Category = "Backpack")
	void CloseBackpack();

	/** Refresh the inventory display - call this to update all slots */
	UFUNCTION(BlueprintCallable, Category = "Backpack")
	void RefreshInventory();

	/** Delegate broadcast when backpack is closed */
	UPROPERTY(BlueprintAssignable, Category = "Backpack")
	FOnBackpackClosed OnBackpackClosed;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void OnCloseButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> InventoryGrid;

	/** Number of columns in the inventory grid */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Backpack")
	int32 GridColumns = 9;

	/** Total number of inventory slots */
	static constexpr int32 TotalSlotCount = 27;

private:
	UFUNCTION()
	void OnInventoryChanged();

	void BindToInventoryComponent();
	void UpdateAllSlots();
	UInventorySlotWidget* GetOrCreateSlotWidget(int32 SlotIndex);
	
	UFUNCTION()
	void HandleSlotClicked(int32 SlotIndex, int32 InventorySourceID);
	
	UFUNCTION()
	void HandleDragStarted(int32 SlotIndex, int32 InventorySourceID, const FInventorySlot& SlotData);
	
	UFUNCTION()
	void HandleSlotDropped(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID);

	AFungiFieldsCharacter* GetPlayerCharacter() const;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> CachedInventoryComponent;

	UPROPERTY()
	TArray<TObjectPtr<UInventorySlotWidget>> SlotWidgets;

	/** Slot widget class to use */
	UPROPERTY(EditDefaultsOnly, Category = "Backpack")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;
};


