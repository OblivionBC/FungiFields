#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UItemDataAsset.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "UCropDataAsset.generated.h"

/**
 * Data Asset for defining crop properties.
 * Follows data-driven design principles - all crop configuration is external to C++ code.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API UCropDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UCropDataAsset();

	/** Name of the crop */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crop Properties")
	FName CropName;

	/** Base time in seconds to reach full maturity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crop Properties", meta = (ClampMin = "0.0"))
	float GrowthTimeSeconds = 60.0f;

	/** Water units consumed per time period */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crop Properties", meta = (ClampMin = "0.0"))
	float WaterConsumptionRate = 1.0f;

	/** Item added to inventory on harvest */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crop Properties")
	TSoftObjectPtr<UItemDataAsset> HarvestItem;

	/** Base number of items harvested */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crop Properties", meta = (ClampMin = "1"))
	int32 BaseHarvestQuantity = 1;

	/** Meshes for different growth stages (0%, 25%, 50%, 100%) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crop Visuals")
	TArray<TSoftObjectPtr<UStaticMesh>> GrowthMeshes;

	/** Mesh when crop wilts from lack of water */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crop Visuals")
	TSoftObjectPtr<UStaticMesh> WitheredMesh;

	/** Particle effect to spawn when harvesting this crop (Niagara) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crop Visuals")
	TObjectPtr<UNiagaraSystem> HarvestParticleEffect;

	/** Particle effect to spawn when harvesting this crop (Cascade - legacy) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crop Visuals")
	TObjectPtr<UParticleSystem> HarvestParticleEffectCascade;
};