#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/IHarvestableInterface.h"
#include "ACropBase.generated.h"

class UCropGrowthComponent;
class UStaticMeshComponent;
class UCropDataAsset;
class ASoilPlot;
class UItemDataAsset;
struct FHarvestResult;

/**
 * Actor representing a crop that grows over time.
 * Implements IHarvestableInterface for decoupled harvest interaction.
 */
UCLASS()
class FUNGIFIELDS_API ACropBase : public AActor, public IHarvestableInterface
{
	GENERATED_BODY()

public:
	ACropBase();

	virtual void BeginPlay() override;

	// IHarvestableInterface implementation
	virtual FHarvestResult Harvest_Implementation(AActor* Harvester, float ToolPower) override;
	virtual bool CanHarvest_Implementation() const override;
	virtual FText GetHarvestText_Implementation() const override;
	virtual FVector GetActionLocation_Implementation() const override;
	virtual float GetInteractionRange_Implementation() const override;

	// ITooltipProvider implementation
	virtual FText GetTooltipText_Implementation() const override;

	/**
	 * Initialize the crop with crop data and parent soil.
	 * @param InCropData The crop data asset to use for configuration
	 * @param InParentSoil The soil plot this crop is planted on
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop")
	void Initialize(UCropDataAsset* InCropData, ASoilPlot* InParentSoil);

	/**
	 * Get the growth component.
	 * @return The growth component
	 */
	UFUNCTION(BlueprintPure, Category = "Crop")
	UCropGrowthComponent* GetGrowthComponent() const { return GrowthComponent; }

	/**
	 * Get the crop data asset.
	 * @return The crop data asset
	 */
	UFUNCTION(BlueprintPure, Category = "Crop")
	UCropDataAsset* GetCropData() const { return CropDataAsset; }

protected:
	/**
	 * Update the crop mesh based on growth stage.
	 * Called by growth component when stage changes.
	 */
	UFUNCTION()
	void OnGrowthStageChanged(AActor* Crop, float Progress);

	/**
	 * Handle crop fully grown event.
	 */
	UFUNCTION()
	void OnCropFullyGrown(AActor* Crop);

	/**
	 * Handle crop withered event.
	 */
	UFUNCTION()
	void OnCropWithered(AActor* Crop);

	/**
	 * Spawn harvest items at crop location.
	 * @param ItemData The item data asset to spawn
	 * @param Quantity Number of items to spawn
	 */
	void SpawnHarvestItems(UItemDataAsset* ItemData, int32 Quantity);

	/** Growth component managing crop lifecycle */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCropGrowthComponent> GrowthComponent;

	/** Visual representation of the crop */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Configuration data for this crop */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crop Data")
	TObjectPtr<UCropDataAsset> CropDataAsset;

	/** Reference to the parent soil plot */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crop Data")
	TObjectPtr<ASoilPlot> ParentSoil;

	/** Class of item pickup to spawn on harvest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TSubclassOf<AActor> ItemPickupClass;

	UPROPERTY(EditAnywhere, Category = "Crop")
	float HarvestProgress = 0;

	UPROPERTY(EditAnywhere, Category = "Crop")
	float HarvestThreshold = 100.0f;
};