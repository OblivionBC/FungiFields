#include "UShopWidget.h"
#include "UShopStockEntryWidget.h"
#include "UInventorySellEntryWidget.h"
#include "../Components/UShopComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Data/UShopStockDataAsset.h"
#include "../Inventory/FInventorySlot.h"
#include "FungiFields/Attributes/EconomyAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

UShopWidget::UShopWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UShopWidget::OnCloseButtonClicked);
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
			UEconomyAttributeSet::GetGoldAttribute()).RemoveAll(this);
	}
	if (ShopComp)
	{
		ShopComp->OnItemPurchased.RemoveDynamic(this, &UShopWidget::OnItemPurchasedFromShop);
		ShopComp->OnItemSold.RemoveDynamic(this, &UShopWidget::OnItemSoldFromShop);
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

void UShopWidget::SetupShop(UShopComponent* InShopComp, UInventoryComponent* InInventory,
                             UAbilitySystemComponent* InASC)
{
	ShopComp          = InShopComp;
	InventoryComp     = InInventory;
	AbilitySystemComp = InASC;

	if (ShopComp && ShopNameText)
		ShopNameText->SetText(ShopComp->GetShopName());

	if (InventoryComp)
	{
		InventoryComp->OnItemAdded.AddDynamic(this, &UShopWidget::OnInventoryItemAdded);
		InventoryComp->OnItemRemoved.AddDynamic(this, &UShopWidget::OnInventoryItemRemoved);
	}
	if (AbilitySystemComp)
	{
		AbilitySystemComp->GetGameplayAttributeValueChangeDelegate(
			UEconomyAttributeSet::GetGoldAttribute()).AddUObject(this, &UShopWidget::OnGoldChanged);
	}
	if (ShopComp)
	{
		ShopComp->OnItemPurchased.AddDynamic(this, &UShopWidget::OnItemPurchasedFromShop);
		ShopComp->OnItemSold.AddDynamic(this, &UShopWidget::OnItemSoldFromShop);
	}

	RefreshStockPanel();
	RefreshInventoryPanel();
	UpdateGoldDisplay();
}

void UShopWidget::CloseWidget()
{
	RemoveFromParent();
	OnShopWidgetClosed.Broadcast();
}

void UShopWidget::RefreshStockPanel()
{
	if (!ShopStockPanel) return;
	ShopStockPanel->ClearChildren();
	StockEntryWidgets.Empty();

	if (!ShopComp || !StockEntryWidgetClass) return;

	UShopStockDataAsset* StockData = ShopComp->GetStockData();
	if (!StockData) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	for (const FShopStockEntry& Entry : StockData->StockEntries)
	{
		if (!Entry.Item) continue;
		UShopStockEntryWidget* EntryWidget = CreateWidget<UShopStockEntryWidget>(PC, StockEntryWidgetClass);
		if (!EntryWidget) continue;
		EntryWidget->SetupEntry(ShopComp, Entry, InventoryComp, AbilitySystemComp);
		ShopStockPanel->AddChild(EntryWidget);
		StockEntryWidgets.Add(EntryWidget);
	}
}

void UShopWidget::RefreshInventoryPanel()
{
	if (!InventoryPanel) return;
	InventoryPanel->ClearChildren();
	SellEntryWidgets.Empty();

	if (!InventoryComp || !InventorySellEntryWidgetClass) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		UInventorySellEntryWidget* EntryWidget = CreateWidget<UInventorySellEntryWidget>(PC, InventorySellEntryWidgetClass);
		if (!EntryWidget) continue;
		EntryWidget->SetupEntry(ShopComp, InventoryComp, AbilitySystemComp, i);
		InventoryPanel->AddChild(EntryWidget);
		SellEntryWidgets.Add(EntryWidget);
	}
}

void UShopWidget::UpdateGoldDisplay()
{
	if (GoldAmountText && AbilitySystemComp)
	{
		const float Gold = AbilitySystemComp->GetNumericAttribute(UEconomyAttributeSet::GetGoldAttribute());
		GoldAmountText->SetText(FText::Format(
			NSLOCTEXT("ShopWidget", "Gold", "{0} Gold"),
			FText::AsNumber(FMath::FloorToInt(Gold))));
	}

	if (MerchantGoldText && ShopComp)
	{
		MerchantGoldText->SetText(FText::Format(
			NSLOCTEXT("ShopWidget", "MerchantGold", "Shop: {0} G"),
			FText::AsNumber(FMath::FloorToInt(ShopComp->GetMerchantGold()))));
	}
}

void UShopWidget::OnCloseButtonClicked() { CloseWidget(); }

void UShopWidget::OnInventoryItemAdded(UItemDataAsset*, int32, int32)   { RefreshInventoryPanel(); }
void UShopWidget::OnInventoryItemRemoved(UItemDataAsset*, int32, int32) { RefreshInventoryPanel(); }

void UShopWidget::OnItemPurchasedFromShop(UItemDataAsset*, int32, float)
{
	for (UShopStockEntryWidget* W : StockEntryWidgets) { if (W) W->RefreshPrice(); }
	UpdateGoldDisplay();
}

void UShopWidget::OnItemSoldFromShop(UItemDataAsset*, int32, float)
{
	for (UShopStockEntryWidget* W : StockEntryWidgets) { if (W) W->RefreshPrice(); }
	UpdateGoldDisplay();
}

void UShopWidget::OnGoldChanged(const FOnAttributeChangeData& ChangeData)
{
	if (GoldAmountText)
	{
		GoldAmountText->SetText(FText::Format(
			NSLOCTEXT("ShopWidget", "Gold", "{0} Gold"),
			FText::AsNumber(FMath::FloorToInt(ChangeData.NewValue))));
	}
}
