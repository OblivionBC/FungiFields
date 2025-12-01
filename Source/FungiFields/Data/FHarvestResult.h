#pragma once

#include "CoreMinimal.h"
#include "FHarvestResult.generated.h"

class UItemDataAsset;

/**
 * Struct containing the result of a harvest operation.
 * Returned by IHarvestableInterface::Harvest().
 */
USTRUCT(BlueprintType)
struct FUNGIFIELDS_API FHarvestResult
{
	GENERATED_BODY()

	/** Item to add to inventory */
	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	TObjectPtr<UItemDataAsset> HarvestItem = nullptr;

	/** Number of items harvested */
	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	int32 Quantity = 0;

	/** Whether the harvest was successful */
	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	bool bSuccess = false;

	FHarvestResult()
		: HarvestItem(nullptr)
		, Quantity(0)
		, bSuccess(false)
	{
	}
};

