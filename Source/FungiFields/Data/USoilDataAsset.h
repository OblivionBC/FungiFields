#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "USoilDataAsset.generated.h"

/**
 * Data Asset for defining soil properties.
 * Follows data-driven design principles - all soil configuration is external to C++ code.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API USoilDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	USoilDataAsset();

	/** Name of the soil type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Soil Properties")
	FName SoilName;

	/** Growth speed multiplier (1.0 = normal, 1.2 = 20% faster) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Soil Properties", meta = (ClampMin = "0.1"))
	float BaseFertility = 1.0f;

	/** Reduces water evaporation rate (1.0 = normal, >1.0 = slower evaporation) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Soil Properties", meta = (ClampMin = "0.1"))
	float WaterRetentionMultiplier = 1.0f;

	/** Chance (0-1) to double crop yield on harvest */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Soil Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float YieldChance = 0.0f;

	/** Maximum water capacity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Soil Properties", meta = (ClampMin = "0.0"))
	float MaxWaterLevel = 100.0f;

	/** Material for the soil mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Soil Visuals")
	TObjectPtr<UMaterialInterface> SoilMaterial;
};