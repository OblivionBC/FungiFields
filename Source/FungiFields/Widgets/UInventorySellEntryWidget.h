#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UInventorySellEntryWidget.generated.h"

class UShopComponent;
class UInventoryComponent;
class UAbilitySystemComponent;
class UItemDataAsset;
class UImage;
class UTextBlock;
class UButton;
class USpinBox;

/**
 * Single sell entry for the shop's inventory panel.
 * Collapses itself for empty slots. Abstract — create a Blueprint child and bind the named widgets.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UInventorySellEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sell Entry")
	void SetupEntry(UShopComponent* InShopComp, UInventoryComponent* InInventory,
	                UAbilitySystemComponent* InASC, int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Sell Entry")
	void RefreshPrice();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION() void OnSellButtonClicked();

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     ItemIcon;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> ItemNameText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> SellPriceText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USpinBox>   QuantitySpinner;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton>    SellButton;

private:
	UPROPERTY() TObjectPtr<UShopComponent>          ShopComp;
	UPROPERTY() TObjectPtr<UInventoryComponent>     InventoryComp;
	UPROPERTY() TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;
	UPROPERTY() TObjectPtr<const UItemDataAsset>    CachedItem;
	int32 SlotIndex = INDEX_NONE;
};
