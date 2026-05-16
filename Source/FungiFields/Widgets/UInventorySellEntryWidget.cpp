#include "UInventorySellEntryWidget.h"
#include "../Components/UShopComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Subsystems/UPricingSubsystem.h"
#include "../Data/UShopStockDataAsset.h"
#include "../Data/FShopStockEntry.h"
#include "../Inventory/FInventorySlot.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SpinBox.h"

void UInventorySellEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (SellButton)
		SellButton->OnClicked.AddDynamic(this, &UInventorySellEntryWidget::OnSellButtonClicked);
}

void UInventorySellEntryWidget::SetupEntry(UShopComponent* InShopComp, UInventoryComponent* InInventory,
                                            UAbilitySystemComponent* InASC, int32 InSlotIndex)
{
	ShopComp          = InShopComp;
	InventoryComp     = InInventory;
	AbilitySystemComp = InASC;
	SlotIndex         = InSlotIndex;
	CachedItem        = nullptr;

	if (!InventoryComp)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FInventorySlot& InvSlot = Slots[SlotIndex];
	CachedItem = InvSlot.ItemDefinition;

	if (ItemNameText)
		ItemNameText->SetText(InvSlot.ItemDefinition->ItemName);

	if (ItemIcon && InvSlot.ItemDefinition->ItemIcon)
		ItemIcon->SetBrushFromTexture(InvSlot.ItemDefinition->ItemIcon);

	if (QuantitySpinner)
	{
		QuantitySpinner->SetMinValue(1.f);
		QuantitySpinner->SetMaxValue(static_cast<float>(InvSlot.Count));
		QuantitySpinner->SetValue(1.f);
	}

	SetVisibility(ESlateVisibility::Visible);
	RefreshPrice();
}

void UInventorySellEntryWidget::RefreshPrice()
{
	if (!CachedItem || !SellPriceText) return;

	float BaseSellPrice = CachedItem->BaseSellPrice;

	if (ShopComp)
	{
		if (UShopStockDataAsset* StockData = ShopComp->GetStockData())
		{
			for (const FShopStockEntry& Entry : StockData->StockEntries)
			{
				if (Entry.Item == CachedItem)
				{
					BaseSellPrice = Entry.BasePrice * Entry.SellPriceMultiplier;
					break;
				}
			}
		}
		if (BaseSellPrice <= 0.f)
			BaseSellPrice = ShopComp->GetFallbackSellGoldPerUnit();
	}

	float DisplayPrice = BaseSellPrice;
	if (UWorld* World = GetWorld())
	{
		if (UPricingSubsystem* Pricing = World->GetSubsystem<UPricingSubsystem>())
			DisplayPrice = Pricing->GetSellPrice(CachedItem, BaseSellPrice);
	}

	SellPriceText->SetText(FText::Format(
		NSLOCTEXT("SellEntry", "SellPrice", "{0} Gold / unit"),
		FText::AsNumber(FMath::FloorToInt(DisplayPrice))));
}

void UInventorySellEntryWidget::OnSellButtonClicked()
{
	if (!ShopComp || !InventoryComp || !AbilitySystemComp || SlotIndex == INDEX_NONE) return;
	const int32 Quantity = QuantitySpinner ? FMath::Max(1, FMath::RoundToInt(QuantitySpinner->GetValue())) : 1;
	ShopComp->TrySellItem(InventoryComp, AbilitySystemComp, SlotIndex, Quantity);
}
