#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FShopStockEntry.h"
#include "UShopStockDataAsset.generated.h"

/**
 * Defines the complete stock list for a single shop type.
 * Assign one asset per shop actor in the editor (e.g. DA_ShopStock_General, DA_ShopStock_Seeds).
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API UShopStockDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** All items available in this shop, with their pricing and restock configuration. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shop Stock")
	TArray<FShopStockEntry> StockEntries;
};
