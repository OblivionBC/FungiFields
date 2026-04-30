#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Data/FShopStockEntry.h"
#include "UShopStockEntryWidget.generated.h"

class AShopActor;
class UInventoryComponent;
class UAbilitySystemComponent;
class UImage;
class UTextBlock;
class UButton;
class USpinBox;

/**
 * Represents a single buyable item row in the shop stock panel.
 * Displays icon, name, and current dynamic buy price.
 * The Buy button calls AShopActor::TryBuyItem with the configured quantity.
 *
 * This widget is Abstract — create a Blueprint child and bind the named widget properties.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UShopStockEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Initialises the entry with live references used during buy transactions.
	 * Call this immediately after CreateWidget.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop Stock Entry")
	void SetupEntry(AShopActor* InShopActor, const FShopStockEntry& InEntry,
	                UInventoryComponent* InInventory, UAbilitySystemComponent* InASC);

	/** Refreshes the displayed buy price from UPricingSubsystem (call after demand changes). */
	UFUNCTION(BlueprintCallable, Category = "Shop Stock Entry")
	void RefreshPrice();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnBuyButtonClicked();

	/** Item icon — bind to an Image in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	/** Item name text — bind to a TextBlock in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	/** Current dynamic buy price text — bind to a TextBlock in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BuyPriceText;

	/** Quantity to purchase — bind to a SpinBox in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpinBox> QuantitySpinner;

	/** Triggers the buy transaction — bind to a Button in Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BuyButton;

private:
	UPROPERTY()
	TObjectPtr<AShopActor> ShopActor;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;

	FShopStockEntry CachedEntry;
};
