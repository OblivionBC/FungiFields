#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "UShopWidget.generated.h"

class UShopComponent;
class UInventoryComponent;
class UAbilitySystemComponent;
class UShopStockEntryWidget;
class UInventorySellEntryWidget;
class UScrollBox;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopWidgetClosed);

/**
 * Two-panel shop UI.
 *   Left  — shop stock with dynamic buy prices and quantity spinners.
 *   Right — player inventory; each occupied slot renders as a UInventorySellEntryWidget.
 *
 * Reactively updates gold via GAS attribute delegate and refreshes inventory slots via
 * UInventoryComponent::OnItemAdded / OnItemRemoved.
 *
 * Abstract — create a Blueprint child and bind the named widget properties.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UShopWidget(const FObjectInitializer& ObjectInitializer);

	/** Binds to a shop component, inventory, and ability system. Call after CreateWidget, before AddToViewport. */
	UFUNCTION(BlueprintCallable, Category = "Shop Widget")
	void SetupShop(UShopComponent* InShopComp, UInventoryComponent* InInventory,
	               UAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintCallable, Category = "Shop Widget")
	void CloseWidget();

	UPROPERTY(BlueprintAssignable, Category = "Shop Widget")
	FOnShopWidgetClosed OnShopWidgetClosed;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))         TObjectPtr<UScrollBox>  ShopStockPanel;
	UPROPERTY(meta = (BindWidget))         TObjectPtr<UScrollBox>  InventoryPanel;
	UPROPERTY(meta = (BindWidget))         TObjectPtr<UTextBlock>  GoldAmountText;
	UPROPERTY(meta = (BindWidget))         TObjectPtr<UTextBlock>  ShopNameText;
	UPROPERTY(meta = (BindWidget))         TObjectPtr<UButton>     CloseButton;
	/** Optional — shows the merchant's current gold pool so the player knows their sell limit. */
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock>  MerchantGoldText;

	UPROPERTY(EditDefaultsOnly, Category = "Shop Widget")
	TSubclassOf<UShopStockEntryWidget> StockEntryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shop Widget")
	TSubclassOf<UInventorySellEntryWidget> InventorySellEntryWidgetClass;

private:
	void RefreshStockPanel();
	void RefreshInventoryPanel();
	void UpdateGoldDisplay();

	UFUNCTION() void OnCloseButtonClicked();
	UFUNCTION() void OnInventoryItemAdded(UItemDataAsset* Item, int32 Amount, int32 NewTotal);
	UFUNCTION() void OnInventoryItemRemoved(UItemDataAsset* Item, int32 Amount, int32 NewTotal);
	UFUNCTION() void OnItemPurchasedFromShop(UItemDataAsset* Item, int32 Quantity, float TotalCost);
	UFUNCTION() void OnItemSoldFromShop(UItemDataAsset* Item, int32 Quantity, float TotalProceeds);

	void OnGoldChanged(const FOnAttributeChangeData& ChangeData);

	UPROPERTY() TObjectPtr<UShopComponent>        ShopComp;
	UPROPERTY() TObjectPtr<UInventoryComponent>   InventoryComp;
	UPROPERTY() TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;

	UPROPERTY() TArray<TObjectPtr<UShopStockEntryWidget>>    StockEntryWidgets;
	UPROPERTY() TArray<TObjectPtr<UInventorySellEntryWidget>> SellEntryWidgets;
};
