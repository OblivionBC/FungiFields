#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Data/FShopStockEntry.h"
#include "UShopStockEntryWidget.generated.h"

class UShopComponent;
class UInventoryComponent;
class UAbilitySystemComponent;
class UImage;
class UTextBlock;
class UButton;
class USpinBox;

/**
 * Single buyable-item row in the shop stock panel.
 * Abstract — create a Blueprint child and bind the named widget properties.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UShopStockEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Shop Stock Entry")
	void SetupEntry(UShopComponent* InShopComp, const FShopStockEntry& InEntry,
	                UInventoryComponent* InInventory, UAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintCallable, Category = "Shop Stock Entry")
	void RefreshPrice();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION() void OnBuyButtonClicked();

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     ItemIcon;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> ItemNameText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> BuyPriceText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USpinBox>   QuantitySpinner;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton>    BuyButton;

private:
	UPROPERTY() TObjectPtr<UShopComponent>        ShopComp;
	UPROPERTY() TObjectPtr<UInventoryComponent>   InventoryComp;
	UPROPERTY() TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;
	FShopStockEntry CachedEntry;
};
