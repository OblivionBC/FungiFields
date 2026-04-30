#include "UShopWidget.h"
#include "UShopStockEntryWidget.h"
#include "UInventorySlotWidget.h"
#include "../Actors/AShopActor.h"
#include "../Components/InventoryComponent.h"
#include "../Subsystems/UPricingSubsystem.h"
#include "../Data/UShopStockDataAsset.h"
#include "../Inventory/FInventorySlot.h"
#include "FungiFields/Attributes/EconomyAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SpinBox.h"

UShopWidget::UShopWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UShopWidget::OnCloseButtonClicked);
	}

	if (SellButton)
	{
		SellButton->OnClicked.AddDynamic(this, &UShopWidget::OnSellButtonClicked);
	}

	if (SellQuantitySpinner)
	{
		SellQuantitySpinner->SetMinValue(1.f);
		SellQuantitySpinner->SetValue(1.f);
	}
}

void UShopWidget::NativeDestruct()
{
	if (InventoryComp)
	{
		InventoryComp->OnItemAdded.RemoveDynamic(this, &UShopWidget::OnInventoryItemAdded);
		InventoryComp->OnItemRemoved.RemoveDynamic(this, &UShopWidget::OnInventoryItemRemoved);
	}

	if (AbilitySystemComp)
	{
		AbilitySystemComp->GetGameplayAttributeValueChangeDelegate(
			UEconomyAttributeSet::GetGoldAttribute()
		).RemoveAll(this);
	}

	if (ShopActor)
	{
		ShopActor->OnItemPurchased.RemoveDynamic(this, &UShopWidget::OnItemPurchasedFromShop);
	}

	Super::NativeDestruct();
}

FReply UShopWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseWidget();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UShopWidget::SetupShop(AShopActor* InShopActor, UInventoryComponent* InInventory,
                             UAbilitySystemComponent* InASC)
{
	ShopActor         = InShopActor;
	InventoryComp     = InInventory;
	AbilitySystemComp = InASC;

	if (ShopActor && ShopNameText)
	{
		ShopNameText->SetText(ShopActor->GetShopName());
	}

	// Bind inventory delegates so panels stay current without polling.
	if (InventoryComp)
	{
		InventoryComp->OnItemAdded.AddDynamic(this, &UShopWidget::OnInventoryItemAdded);
		InventoryComp->OnItemRemoved.AddDynamic(this, &UShopWidget::OnInventoryItemRemoved);
	}

	// Bind GAS Gold attribute change for live gold display.
	if (AbilitySystemComp)
	{
		AbilitySystemComp->GetGameplayAttributeValueChangeDelegate(
			UEconomyAttributeSet::GetGoldAttribute()
		).AddUObject(this, &UShopWidget::OnGoldChanged);
	}

	// Refresh stock prices after each purchase so demand changes are reflected immediately.
	if (ShopActor)
	{
		ShopActor->OnItemPurchased.AddDynamic(this, &UShopWidget::OnItemPurchasedFromShop);
	}

	RefreshStockPanel();
	RefreshInventoryPanel();
	UpdateGoldDisplay();
	UpdateSellSelectionDisplay();
}

void UShopWidget::CloseWidget()
{
	RemoveFromParent();
	OnShopWidgetClosed.Broadcast();
}

// ── UI construction ───────────────────────────────────────────────────────────

void UShopWidget::RefreshStockPanel()
{
	if (!ShopStockPanel)
	{
		return;
	}

	ShopStockPanel->ClearChildren();
	StockEntryWidgets.Empty();

	if (!ShopActor || !StockEntryWidgetClass)
	{
		return;
	}

	UShopStockDataAsset* StockData = ShopActor->GetStockData();
	if (!StockData)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	for (const FShopStockEntry& Entry : StockData->StockEntries)
	{
		if (!Entry.Item)
		{
			continue;
		}

		UShopStockEntryWidget* EntryWidget = CreateWidget<UShopStockEntryWidget>(PC, StockEntryWidgetClass);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->SetupEntry(ShopActor, Entry, InventoryComp, AbilitySystemComp);
		ShopStockPanel->AddChild(EntryWidget);
		StockEntryWidgets.Add(EntryWidget);
	}
}

void UShopWidget::RefreshInventoryPanel()
{
	if (!InventoryPanel)
	{
		return;
	}

	InventoryPanel->ClearChildren();
	InventorySlotWidgets.Empty();
	SelectedSellSlotIndex = INDEX_NONE;

	if (!InventoryComp || !InventorySlotWidgetClass)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(PC, InventorySlotWidgetClass);
		if (!SlotWidget)
		{
			continue;
		}

		SlotWidget->SetSlotData(Slots[i], i);
		SlotWidget->SetInventorySource(0);
		SlotWidget->OnSlotClicked.AddDynamic(this, &UShopWidget::HandleInventorySlotClicked);

		InventoryPanel->AddChild(SlotWidget);
		InventorySlotWidgets.Add(SlotWidget);
	}

	UpdateSellSelectionDisplay();
}

void UShopWidget::UpdateGoldDisplay()
{
	if (!GoldAmountText || !AbilitySystemComp)
	{
		return;
	}

	const float Gold = AbilitySystemComp->GetNumericAttribute(UEconomyAttributeSet::GetGoldAttribute());
	GoldAmountText->SetText(FText::Format(
		NSLOCTEXT("ShopWidget", "Gold", "{0} Gold"),
		FText::AsNumber(FMath::FloorToInt(Gold))
	));
}

void UShopWidget::UpdateSellSelectionDisplay()
{
	const bool bHasSelection = SelectedSellSlotIndex != INDEX_NONE && InventoryComp;

	if (SellButton)
	{
		SellButton->SetIsEnabled(bHasSelection);
	}

	if (!bHasSelection)
	{
		if (SelectedItemNameText)
		{
			SelectedItemNameText->SetText(NSLOCTEXT("ShopWidget", "NoSelection", "Select an item to sell"));
		}
		if (SelectedSellPriceText)
		{
			SelectedSellPriceText->SetText(FText::GetEmpty());
		}
		return;
	}

	const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
	if (!Slots.IsValidIndex(SelectedSellSlotIndex))
	{
		SelectedSellSlotIndex = INDEX_NONE;
		UpdateSellSelectionDisplay();
		return;
	}

	const FInventorySlot& CurrSlot = Slots[SelectedSellSlotIndex];
	if (CurrSlot.IsEmpty())
	{
		SelectedSellSlotIndex = INDEX_NONE;
		UpdateSellSelectionDisplay();
		return;
	}

	if (SelectedItemNameText)
	{
		SelectedItemNameText->SetText(CurrSlot.ItemDefinition->ItemName);
	}

	if (SelectedSellPriceText && ShopActor)
	{
		float BaseSellPrice = 0.0f;
		bool bFoundInStock = false;

		UShopStockDataAsset* StockData = ShopActor->GetStockData();
		if (StockData)
		{
			for (const FShopStockEntry& Entry : StockData->StockEntries)
			{
				if (Entry.Item == CurrSlot.ItemDefinition.Get())
				{
					BaseSellPrice = Entry.BasePrice * Entry.SellPriceMultiplier;
					bFoundInStock = true;
					break;
				}
			}
		}

		if (!bFoundInStock)
		{
			BaseSellPrice = (CurrSlot.ItemDefinition->BaseSellPrice > 0.0f)
				? CurrSlot.ItemDefinition->BaseSellPrice
				: ShopActor->GetFallbackSellGoldPerUnit();
		}

		float DisplaySellPrice = BaseSellPrice;
		if (UWorld* World = GetWorld())
		{
			if (UPricingSubsystem* Pricing = World->GetSubsystem<UPricingSubsystem>())
			{
				DisplaySellPrice = Pricing->GetSellPrice(CurrSlot.ItemDefinition.Get(), BaseSellPrice);
			}
		}

		SelectedSellPriceText->SetText(FText::Format(
			NSLOCTEXT("ShopWidget", "SellPrice", "{0} Gold / unit"),
			FText::AsNumber(FMath::FloorToInt(DisplaySellPrice))
		));
	}
}

// ── Delegate handlers ─────────────────────────────────────────────────────────

void UShopWidget::OnCloseButtonClicked()
{
	CloseWidget();
}

void UShopWidget::OnSellButtonClicked()
{
	if (!ShopActor || !InventoryComp || !AbilitySystemComp || SelectedSellSlotIndex == INDEX_NONE)
	{
		return;
	}

	const int32 Quantity = SellQuantitySpinner
		? FMath::Max(1, FMath::RoundToInt(SellQuantitySpinner->GetValue()))
		: 1;

	ShopActor->TrySellItem(InventoryComp, AbilitySystemComp, SelectedSellSlotIndex, Quantity);
}

void UShopWidget::OnInventoryItemAdded(UItemDataAsset* Item, int32 Amount, int32 NewTotal)
{
	RefreshInventoryPanel();
}

void UShopWidget::OnInventoryItemRemoved(UItemDataAsset* Item, int32 Amount, int32 NewTotal)
{
	RefreshInventoryPanel();
}

void UShopWidget::HandleInventorySlotClicked(int32 SlotIndex, int32 InventorySourceID)
{
	if (!InventoryComp)
	{
		return;
	}

	const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
	{
		SelectedSellSlotIndex = INDEX_NONE;
	}
	else
	{
		SelectedSellSlotIndex = SlotIndex;
	}

	if (SellQuantitySpinner)
	{
		SellQuantitySpinner->SetValue(1.f);
	}

	UpdateSellSelectionDisplay();
}

void UShopWidget::OnItemPurchasedFromShop(UItemDataAsset* Item, int32 Quantity, float TotalCost)
{
	// Demand changed — refresh all stock entry prices to reflect the new multiplier.
	for (UShopStockEntryWidget* EntryWidget : StockEntryWidgets)
	{
		if (EntryWidget)
		{
			EntryWidget->RefreshPrice();
		}
	}
}

void UShopWidget::OnGoldChanged(const FOnAttributeChangeData& ChangeData)
{
	if (GoldAmountText)
	{
		GoldAmountText->SetText(FText::Format(
			NSLOCTEXT("ShopWidget", "Gold", "{0} Gold"),
			FText::AsNumber(FMath::FloorToInt(ChangeData.NewValue))
		));
	}
}
