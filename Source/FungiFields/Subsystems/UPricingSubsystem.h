#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UPricingSubsystem.generated.h"

class UItemDataAsset;

/**
 * Per-item rolling 3-day record of player-driven buy/sell activity.
 * Index [2] is always the current day; [0] is the oldest.
 * Not a USTRUCT — internal only, never reflected.
 */
struct FItemDemandRecord
{
	int32 DailySales[3]     = {0, 0, 0};
	int32 DailyPurchases[3] = {0, 0, 0};

	/** Shifts the window forward by one day and clears the current-day bucket. */
	void AdvanceDay();

	int32 TotalSales() const;
	int32 TotalPurchases() const;
};

/**
 * World subsystem that tracks per-item demand over a rolling 3-day window and
 * applies a linear price multiplier clamped between 0.5× and 2.0× base price.
 *
 * High net purchases  → multiplier > 1  → higher buy price, lower sell returns.
 * High net sales      → multiplier < 1  → lower buy price, higher sell returns.
 *
 * Call OnDayAdvanced() from the day-cycle system whenever a new in-game day begins.
 */
UCLASS()
class FUNGIFIELDS_API UPricingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Returns the dynamic buy price for an item given its configured base price. */
	UFUNCTION(BlueprintCallable, Category = "Pricing")
	float GetBuyPrice(const UItemDataAsset* Item, float BasePrice) const;

	/** Returns the dynamic sell price for an item given its configured base sell price. */
	UFUNCTION(BlueprintCallable, Category = "Pricing")
	float GetSellPrice(const UItemDataAsset* Item, float BaseSellPrice) const;

	/** Records that the player sold Quantity units of Item to the shop. */
	UFUNCTION(BlueprintCallable, Category = "Pricing")
	void RecordSale(const UItemDataAsset* Item, int32 Quantity);

	/** Records that the player purchased Quantity units of Item from the shop. */
	UFUNCTION(BlueprintCallable, Category = "Pricing")
	void RecordPurchase(const UItemDataAsset* Item, int32 Quantity);

	/** Advances the demand window by one day. All per-day buckets shift; the current-day bucket resets to zero. */
	UFUNCTION(BlueprintCallable, Category = "Pricing")
	void OnDayAdvanced();

private:
	/** Computes the price multiplier for Item based on its 3-day demand history. */
	float ComputeMultiplier(const UItemDataAsset* Item) const;

	/** Demand records keyed on the item asset's FName, which is unique per loaded data asset. */
	TMap<FName, FItemDemandRecord> DemandData;

	/**
	 * How many net purchases (buys − sells) over 3 days shift the multiplier by 1×.
	 * Example: DemandScale=10 → 10 net buys gives a 2× buy price.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Pricing", meta = (ClampMin = "1.0"))
	float DemandScale = 10.f;
};
