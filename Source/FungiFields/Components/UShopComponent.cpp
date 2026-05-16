#include "UShopComponent.h"
#include "InventoryComponent.h"
#include "../Widgets/UShopWidget.h"
#include "../Subsystems/UPricingSubsystem.h"
#include "../Subsystems/UDayNightSubsystem.h"
#include "../Data/FShopStockEntry.h"
#include "../Inventory/FInventorySlot.h"
#include "FungiFields/Attributes/EconomyAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

namespace { constexpr int32 ShopMenuZOrder = 10000; }

UShopComponent::UShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UShopComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeStockQuantities();

	if (UWorld* World = GetWorld())
	{
		if (UDayNightSubsystem* DayNight = World->GetSubsystem<UDayNightSubsystem>())
			DayNight->OnNewDayAdvanced.AddDynamic(this, &UShopComponent::HandleNewDayAdvanced);
	}
}

void UShopComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UDayNightSubsystem* DayNight = World->GetSubsystem<UDayNightSubsystem>())
			DayNight->OnNewDayAdvanced.RemoveDynamic(this, &UShopComponent::HandleNewDayAdvanced);
	}
	Super::EndPlay(EndPlayReason);
}

void UShopComponent::InitializeStockQuantities()
{
	CurrentStockQuantities.Empty();
	if (!StockData) return;
	for (const FShopStockEntry& Entry : StockData->StockEntries)
	{
		if (!Entry.Item) continue;
		CurrentStockQuantities.Add(Entry.Item->GetFName(),
			Entry.DailyRestockQuantity < 0 ? -1 : Entry.DailyRestockQuantity);
	}
}

void UShopComponent::HandleNewDayAdvanced()
{
	if (!StockData) return;
	for (const FShopStockEntry& Entry : StockData->StockEntries)
	{
		if (!Entry.Item || Entry.DailyRestockQuantity < 0) continue;
		int32& Stock = CurrentStockQuantities.FindOrAdd(Entry.Item->GetFName(), 0);
		Stock = FMath::Min(Stock + Entry.DailyRestockQuantity, Entry.DailyRestockQuantity * 3);
	}
}

int32 UShopComponent::GetCurrentStock(const UItemDataAsset* Item) const
{
	if (!Item) return 0;
	const int32* Stock = CurrentStockQuantities.Find(Item->GetFName());
	return Stock ? *Stock : 0;
}

void UShopComponent::OpenShopWidget(AActor* Interactor)
{
	if (!ShopWidgetClass || !Interactor) return;

	APawn* Pawn = Cast<APawn>(Interactor);
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC) return;

	UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("UShopComponent: Interactor '%s' has no UInventoryComponent"), *Interactor->GetName());
		return;
	}

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Interactor);
	UAbilitySystemComponent* ASC = ASCInterface ? ASCInterface->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UShopComponent: Interactor '%s' has no UAbilitySystemComponent"), *Interactor->GetName());
		return;
	}

	if (!ShopWidgetInstance)
	{
		ShopWidgetInstance = CreateWidget<UShopWidget>(PC, ShopWidgetClass);
		if (ShopWidgetInstance)
			ShopWidgetInstance->OnShopWidgetClosed.AddDynamic(this, &UShopComponent::OnShopWidgetClosed);
	}

	if (!ShopWidgetInstance) return;

	ShopOpenerController = PC;
	ShopWidgetInstance->SetupShop(this, Inventory, ASC);
	ShopWidgetInstance->AddToViewport(ShopMenuZOrder);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ShopWidgetInstance->TakeWidget());
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}

void UShopComponent::CloseShopWidget()
{
	if (ShopWidgetInstance)
		ShopWidgetInstance->CloseWidget();
}

void UShopComponent::OnShopWidgetClosed()
{
	if (ShopOpenerController.IsValid())
	{
		ShopOpenerController->SetInputMode(FInputModeGameOnly());
		ShopOpenerController->bShowMouseCursor = false;
	}
	ShopOpenerController = nullptr;
	ShopWidgetInstance   = nullptr;
}

bool UShopComponent::TryBuyItem(UInventoryComponent* PlayerInventory, UAbilitySystemComponent* PlayerASC,
                                 const FShopStockEntry& Entry, int32 Quantity)
{
	if (!PlayerInventory || !PlayerASC || !Entry.Item || Quantity <= 0) return false;

	UWorld* World = GetWorld();
	UPricingSubsystem* Pricing = World ? World->GetSubsystem<UPricingSubsystem>() : nullptr;
	if (!Pricing) return false;

	const int32 AvailableStock = GetCurrentStock(Entry.Item);
	if (AvailableStock == 0) return false;
	if (AvailableStock > 0 && Quantity > AvailableStock) return false;

	const float TotalCost = Pricing->GetBuyPrice(Entry.Item, Entry.BasePrice) * static_cast<float>(Quantity);
	if (PlayerASC->GetNumericAttribute(UEconomyAttributeSet::GetGoldAttribute()) < TotalCost) return false;

	if (!PlayerInventory->TryAddItem(Entry.Item, Quantity)) return false;

	PlayerASC->ApplyModToAttribute(UEconomyAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, -TotalCost);
	MerchantGold += TotalCost;

	if (AvailableStock > 0)
	{
		int32& Stock = CurrentStockQuantities.FindOrAdd(Entry.Item->GetFName(), 0);
		Stock = FMath::Max(0, Stock - Quantity);
	}

	Pricing->RecordPurchase(Entry.Item, Quantity);
	OnItemPurchased.Broadcast(Entry.Item, Quantity, TotalCost);
	return true;
}

bool UShopComponent::TrySellItem(UInventoryComponent* PlayerInventory, UAbilitySystemComponent* PlayerASC,
                                  int32 SlotIndex, int32 Quantity)
{
	if (!PlayerInventory || !PlayerASC || Quantity <= 0) return false;

	const TArray<FInventorySlot>& Slots = PlayerInventory->GetInventorySlots();
	if (!Slots.IsValidIndex(SlotIndex)) return false;

	const FInventorySlot& Slot = Slots[SlotIndex];
	if (Slot.IsEmpty() || Slot.Count < Quantity) return false;

	const UItemDataAsset* Item = Slot.ItemDefinition.Get();
	if (!Item) return false;

	UWorld* World = GetWorld();
	UPricingSubsystem* Pricing = World ? World->GetSubsystem<UPricingSubsystem>() : nullptr;
	if (!Pricing) return false;

	float BaseSellPrice = 0.0f;
	if (const FShopStockEntry* Entry = FindStockEntry(Item))
		BaseSellPrice = Entry->BasePrice * Entry->SellPriceMultiplier;
	else
		BaseSellPrice = (Item->BaseSellPrice > 0.0f) ? Item->BaseSellPrice : FallbackSellGoldPerUnit;

	const float TotalProceeds = Pricing->GetSellPrice(Item, BaseSellPrice) * static_cast<float>(Quantity);

	if (!bHasInfiniteGold && MerchantGold < TotalProceeds) return false;

	if (!PlayerInventory->RemoveFromSlot(SlotIndex, Quantity)) return false;

	if (!bHasInfiniteGold) MerchantGold -= TotalProceeds;
	PlayerASC->ApplyModToAttribute(UEconomyAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, TotalProceeds);

	Pricing->RecordSale(Item, Quantity);
	OnItemSold.Broadcast(const_cast<UItemDataAsset*>(Item), Quantity, TotalProceeds);
	return true;
}

const FShopStockEntry* UShopComponent::FindStockEntry(const UItemDataAsset* Item) const
{
	if (!StockData || !Item) return nullptr;
	for (const FShopStockEntry& Entry : StockData->StockEntries)
	{
		if (Entry.Item == Item) return &Entry;
	}
	return nullptr;
}
