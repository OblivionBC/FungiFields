#pragma once

#include "CoreMinimal.h"
#include "UItemDataAsset.h"
#include "USeedDataAsset.generated.h"

class UCropDataAsset;

/**
 * Data Asset for defining seed properties.
 * Extends UItemDataAsset and links to a crop definition to plant.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API USeedDataAsset : public UItemDataAsset
{
	GENERATED_BODY()

public:
	USeedDataAsset();

	/** Crop to plant when this seed is used on soil */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Seed")
	TObjectPtr<UCropDataAsset> CropToPlant;
};