#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UBackpackWidget.generated.h"

class UInventoryComponent;
class AFungiFieldsCharacter;
class UInventorySlotWidget;
class UUniformGridPanel;
struct FInventorySlot;

/**
 * Reusable inventory grid widget. Displays inventory slots with drag-drop support.
 * Attach any UInventoryComponent via SetInventoryComponent; defaults to the owning player's inventory.
 * Does not include a close button — embed this in a parent widget or subclass UPlayerInventoryWidget.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UBackpackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBackpackWidget(const FObjectInitializer& ObjectInitializer);

	/** Refresh the inventory display */
	UFUNCTION(BlueprintCallable, Category = "Backpack")
	void RefreshInventory();

	/**
	 * Override the default player-inventory source with an external component
	 * (e.g. a villager's UInventoryComponent). Call before AddToViewport.
	 * Pass nullptr to revert to the owning player's inventory.
	 */
	UFUNCTION(BlueprintCallable, Category = "Backpack")
	void SetInventoryComponent(UInventoryComponent* InInventory);

protected:
	virtual void NativeConstruct() override;

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

	/** When set, overrides the default player-inventory lookup. */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> ExternalInventoryComponent;

	UPROPERTY()
	TArray<TObjectPtr<UInventorySlotWidget>> SlotWidgets;

	/** Slot widget class to use */
	UPROPERTY(EditDefaultsOnly, Category = "Backpack")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;
};


