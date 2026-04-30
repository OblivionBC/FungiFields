#include "UPricingSubsystem.h"
#include "../Data/UItemDataAsset.h"

// ── FItemDemandRecord ─────────────────────────────────────────────────────────

void FItemDemandRecord::AdvanceDay()
{
	DailySales[0]     = DailySales[1];
	DailySales[1]     = DailySales[2];
	DailySales[2]     = 0;

	DailyPurchases[0] = DailyPurchases[1];
	DailyPurchases[1] = DailyPurchases[2];
	DailyPurchases[2] = 0;
}

int32 FItemDemandRecord::TotalSales() const
{
	return DailySales[0] + DailySales[1] + DailySales[2];
}

int32 FItemDemandRecord::TotalPurchases() const
{
	return DailyPurchases[0] + DailyPurchases[1] + DailyPurchases[2];
}

// ── UPricingSubsystem ─────────────────────────────────────────────────────────

void UPricingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPricingSubsystem::Deinitialize()
{
	DemandData.Empty();
	Super::Deinitialize();
}

float UPricingSubsystem::GetBuyPrice(const UItemDataAsset* Item, float BasePrice) const
{
	return BasePrice * ComputeMultiplier(Item);
}

float UPricingSubsystem::GetSellPrice(const UItemDataAsset* Item, float BaseSellPrice) const
{
	const float Multiplier = ComputeMultiplier(Item);
	// Sell returns move inversely: high demand reduces what the shop pays back.
	return (Multiplier > 0.f) ? (BaseSellPrice / Multiplier) : BaseSellPrice;
}

void UPricingSubsystem::RecordSale(const UItemDataAsset* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return;
	}

	DemandData.FindOrAdd(Item->GetFName()).DailySales[2] += Quantity;
}

void UPricingSubsystem::RecordPurchase(const UItemDataAsset* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return;
	}

	DemandData.FindOrAdd(Item->GetFName()).DailyPurchases[2] += Quantity;
}

void UPricingSubsystem::OnDayAdvanced()
{
	for (auto& Pair : DemandData)
	{
		Pair.Value.AdvanceDay();
	}
}

float UPricingSubsystem::ComputeMultiplier(const UItemDataAsset* Item) const
{
	if (!Item)
	{
		return 1.f;
	}

	const FItemDemandRecord* Record = DemandData.Find(Item->GetFName());
	if (!Record)
	{
		return 1.f;
	}

	const int32 NetDemand = Record->TotalPurchases() - Record->TotalSales();
	return FMath::Clamp(1.0f + static_cast<float>(NetDemand) / DemandScale, 0.5f, 2.0f);
}
