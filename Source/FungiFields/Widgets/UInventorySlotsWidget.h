#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UInventorySlotsWidget.generated.h"

class UInventoryComponent;
class AFungiFieldsCharacter;
class UTextBlock;
class UHorizontalBox;
class UInventorySlotWidget;
struct FInventorySlot;

/**
 * Container widget that manages and displays the first 9 inventory slots.
 * Subscribes to InventoryComponent delegate for event-driven updates.
 * Highlights the equipped slot visually.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UInventorySlotsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SlotsContainer;

	/** Text block to display the name of the currently equipped item above the hotbar */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EquippedItemNameText;

	/** Optional slot widget class (e.g. from Blueprint). If not set, uses UInventorySlotWidget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hotbar")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnInventoryChanged();

	UFUNCTION()
	void OnItemEquipped(UItemDataAsset* Item, int32 SlotIndex);

	void BindToInventoryComponent();
	void UpdateEquippedItemName();

	AFungiFieldsCharacter* GetPlayerCharacter() const;

	void UpdateSlotVisuals();

	UInventorySlotWidget* GetOrCreateSlotWidget(int32 SlotIndex);

	void UpdateSlotWidget(UInventorySlotWidget* SlotWidget, const FInventorySlot& SlotData, int32 SlotIndex, bool bIsEquipped);

	UFUNCTION()
	void HandleSlotClicked(int32 SlotIndex, int32 InventorySourceID);

	UPROPERTY()
	TObjectPtr<UInventoryComponent> CachedInventoryComponent;

	UPROPERTY()
	TArray<TObjectPtr<UInventorySlotWidget>> SlotWidgets;

	static constexpr int32 HotbarSize = 9;
};