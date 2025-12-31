#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotsWidget.generated.h"

class UInventoryComponent;
class AFungiFieldsCharacter;
class UImage;
class UTextBlock;
class UBorder;
class UHorizontalBox;
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

	UWidget* GetOrCreateSlotWidget(int32 SlotIndex);

	void UpdateSlotWidget(UWidget* SlotWidget, const FInventorySlot& SlotData, int32 SlotIndex, bool bIsEquipped);

	UPROPERTY()
	TObjectPtr<UInventoryComponent> CachedInventoryComponent;

	UPROPERTY()
	TArray<TObjectPtr<UWidget>> SlotWidgets;

	static constexpr int32 HotbarSize = 9;
};