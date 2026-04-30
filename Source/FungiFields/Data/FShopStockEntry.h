#pragma once

#include "CoreMinimal.h"
#include "UItemDataAsset.h"
#include "FShopStockEntry.generated.h"

/**
 * Defines a single line of stock in a shop.
 * Instances live inside UShopStockDataAsset and drive all pricing and restock behaviour.
 */
USTRUCT(BlueprintType)
struct FUNGIFIELDS_API FShopStockEntry
{
	GENERATED_BODY()

	/** Item available for purchase. Must be assigned; a null entry is silently skipped at runtime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stock")
	TObjectPtr<UItemDataAsset> Item = nullptr;

	/** Gold cost for the player to buy one unit. Modified dynamically by UPricingSubsystem. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stock", meta = (ClampMin = "0.0"))
	float BasePrice = 0.0f;

	/** Fraction of BasePrice the player receives when selling this item back to the shop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stock", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SellPriceMultiplier = 0.5f;

	/** Units restocked per in-game day. -1 means the shop always has unlimited supply. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stock")
	int32 DailyRestockQuantity = -1;
};
