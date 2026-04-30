#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "UShopWidget.generated.h"

class AShopActor;
class UInventoryComponent;
class UAbilitySystemComponent;
class UShopStockEntryWidget;
class UInventorySlotWidget;
class UScrollBox;
class UTextBlock;
class UButton;
class USpinBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopWidgetClosed);

/**
 * Two-panel shop UI.
 *   Left  — shop stock with dynamic buy prices and quantity spinners.
 *   Right — player inventory with per-slot sell prices; a selection-based sell control
 *           sits at the bottom of the sell panel.
 *
 * Reactively updates gold display via GAS attribute change delegate and refreshes
 * inventory slots via UInventoryComponent::OnItemAdded / OnItemRemoved.
 *
 * This widget is Abstract — create a Blueprint child and bind the named widget properties.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UShopWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Binds the widget to a shop, inventory, and ability system component.
	 * Call this immediately after CreateWidget and before AddToViewport.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop Widget")
	void SetupShop(AShopActor* InShopActor, UInventoryComponent* InInventory,
	               UAbilitySystemComponent* InASC);

	/** Removes the widget from viewport and broadcasts OnShopWidgetClosed. */
	UFUNCTION(BlueprintCallable, Category = "Shop Widget")
	void CloseWidget();

	/** Broadcast when the widget is closed, so AShopActor can restore input mode. */
	UPROPERTY(BlueprintAssignable, Category = "Shop Widget")
	FOnShopWidgetClosed OnShopWidgetClosed;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	// ── BindWidget — must exist in the Blueprint child ────────────────────────

	/** Container for UShopStockEntryWidget rows (left panel). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ShopStockPanel;

	/** Container for UInventorySlotWidget rows (right panel). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> InventoryPanel;

	/** Displays the player's current Gold amount. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldAmountText;

	/** Displays ShopName. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ShopNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	/** Shows the name of the currently selected sell slot's item. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SelectedItemNameText;

	/** Shows the sell price per unit for the currently selected item. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SelectedSellPriceText;

	/** How many units to sell from the selected slot. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpinBox> SellQuantitySpinner;

	/** Executes the sell transaction for the selected slot and quantity. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SellButton;

	// ── Widget class references — assign in Blueprint child ───────────────────

	/** Blueprint subclass of UShopStockEntryWidget to instantiate for each stock row. */
	UPROPERTY(EditDefaultsOnly, Category = "Shop Widget")
	TSubclassOf<UShopStockEntryWidget> StockEntryWidgetClass;

	/** Blueprint subclass of UInventorySlotWidget to instantiate for each inventory row. */
	UPROPERTY(EditDefaultsOnly, Category = "Shop Widget")
	TSubclassOf<UInventorySlotWidget> InventorySlotWidgetClass;

private:
	// ── UI construction ───────────────────────────────────────────────────────

	void RefreshStockPanel();
	void RefreshInventoryPanel();
	void UpdateGoldDisplay();
	void UpdateSellSelectionDisplay();

	// ── Delegate handlers ─────────────────────────────────────────────────────

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void OnSellButtonClicked();

	UFUNCTION()
	void OnInventoryItemAdded(UItemDataAsset* Item, int32 Amount, int32 NewTotal);

	UFUNCTION()
	void OnInventoryItemRemoved(UItemDataAsset* Item, int32 Amount, int32 NewTotal);

	UFUNCTION()
	void HandleInventorySlotClicked(int32 SlotIndex, int32 InventorySourceID);

	UFUNCTION()
	void OnItemPurchasedFromShop(UItemDataAsset* Item, int32 Quantity, float TotalCost);

	/** GAS attribute change callback — not a UFUNCTION, uses AddUObject binding. */
	void OnGoldChanged(const FOnAttributeChangeData& ChangeData);

	// ── State ─────────────────────────────────────────────────────────────────

	UPROPERTY()
	TObjectPtr<AShopActor> ShopActor;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;

	UPROPERTY()
	TArray<TObjectPtr<UShopStockEntryWidget>> StockEntryWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UInventorySlotWidget>> InventorySlotWidgets;

	/** Inventory slot index currently selected for selling. INDEX_NONE when nothing is selected. */
	int32 SelectedSellSlotIndex = INDEX_NONE;
};
