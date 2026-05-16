#include "UShopStockEntryWidget.h"
#include "../Components/UShopComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Subsystems/UPricingSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SpinBox.h"

void UShopStockEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (BuyButton)
		BuyButton->OnClicked.AddDynamic(this, &UShopStockEntryWidget::OnBuyButtonClicked);
}

void UShopStockEntryWidget::SetupEntry(UShopComponent* InShopComp, const FShopStockEntry& InEntry,
                                        UInventoryComponent* InInventory, UAbilitySystemComponent* InASC)
{
	ShopComp          = InShopComp;
	CachedEntry       = InEntry;
	InventoryComp     = InInventory;
	AbilitySystemComp = InASC;

	if (!InEntry.Item) return;

	if (ItemNameText)
		ItemNameText->SetText(InEntry.Item->ItemName);

	if (ItemIcon && InEntry.Item->ItemIcon)
		ItemIcon->SetBrushFromTexture(InEntry.Item->ItemIcon);

	if (QuantitySpinner)
	{
		QuantitySpinner->SetMinValue(1.f);
		QuantitySpinner->SetValue(1.f);
	}

	RefreshPrice();
}

void UShopStockEntryWidget::RefreshPrice()
{
	if (!CachedEntry.Item || !BuyPriceText) return;

	float DisplayPrice = CachedEntry.BasePrice;
	if (UWorld* World = GetWorld())
	{
		if (UPricingSubsystem* Pricing = World->GetSubsystem<UPricingSubsystem>())
			DisplayPrice = Pricing->GetBuyPrice(CachedEntry.Item, CachedEntry.BasePrice);
	}

	BuyPriceText->SetText(FText::Format(
		NSLOCTEXT("ShopEntry", "BuyPrice", "{0} Gold"),
		FText::AsNumber(FMath::CeilToInt(DisplayPrice))));
}

void UShopStockEntryWidget::OnBuyButtonClicked()
{
	if (!ShopComp || !InventoryComp || !AbilitySystemComp || !CachedEntry.Item) return;
	const int32 Quantity = QuantitySpinner ? FMath::Max(1, FMath::RoundToInt(QuantitySpinner->GetValue())) : 1;
	ShopComp->TryBuyItem(InventoryComp, AbilitySystemComp, CachedEntry, Quantity);
}
