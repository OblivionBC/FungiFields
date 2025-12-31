#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/IFarmableInterface.h"
#include "../ENUM/ESoilState.h"
#include "ASoilPlot.generated.h"

class USoilComponent;
class UStaticMeshComponent;
class USceneComponent;
class USoilDataAsset;
class USoilContainerDataAsset;
class ACropBase;
class UCropDataAsset;
class UMaterialInstanceDynamic;

/**
 * Actor representing a plot of soil that can be tilled, watered, and have crops planted on it.
 * Implements IFarmableInterface for decoupled tool interaction.
 */
UCLASS()
class FUNGIFIELDS_API ASoilPlot : public AActor, public IFarmableInterface
{
	GENERATED_BODY()

public:
	ASoilPlot();

	virtual void BeginPlay() override;

	// IFarmableInterface implementation
	virtual bool InteractTool_Implementation(EToolType ToolType, AActor* Instigator, float ToolPower) override;
	virtual bool CanAcceptSeed_Implementation() const override;
	virtual bool PlantSeed_Implementation(UCropDataAsset* CropToPlant, AActor* Planter) override;
	virtual FText GetInteractionText_Implementation() const override;
	virtual bool CanInteractWithTool_Implementation(EToolType ToolType, AActor* Interactor) const override;
	virtual FVector GetActionLocation_Implementation() const override;
	virtual float GetInteractionRange_Implementation() const override;
	virtual bool CanAcceptSoilBag_Implementation(class UItemDataAsset* SoilBagItem) const override;
	virtual bool AddSoilFromBag_Implementation(class UItemDataAsset* SoilBagItem) override;

	// ITooltipProvider implementation
	virtual FText GetTooltipText_Implementation() const override;

	/**
	 * Initialize the soil plot with container and soil data assets.
	 * @param InContainerData The container data asset to use for the container mesh
	 * @param InSoilData The soil data asset to use for configuration, or nullptr for empty plot
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil Plot")
	void Initialize(USoilContainerDataAsset* InContainerData, USoilDataAsset* InSoilData);

	/**
	 * Initialize the soil plot with soil data asset only (uses existing container data).
	 * Can be called with nullptr for empty plots (container only).
	 * This is for backward compatibility - use the two-parameter Initialize instead.
	 * @param InSoilData The soil data asset to use for configuration, or nullptr for empty plot
	 */
	void Initialize(USoilDataAsset* InSoilData);

	/**
	 * Get the soil component.
	 * @return The soil component
	 */
	UFUNCTION(BlueprintPure, Category = "Soil Plot")
	USoilComponent* GetSoilComponent() const { return SoilComponent; }

protected:
	/**
	 * Update visual representation based on soil state.
	 * Called when soil is tilled or water level changes.
	 */
	UFUNCTION()
	void UpdateVisuals();

	/**
	 * Handler for soil tilled delegate.
	 */
	UFUNCTION()
	void OnSoilTilled(AActor* Soil);

	/**
	 * Handler for crop planted delegate.
	 */
	UFUNCTION()
	void OnCropPlanted(AActor* Soil, ACropBase* Crop);

	/**
	 * Handler for crop removed delegate.
	 */
	UFUNCTION()
	void OnCropRemoved(AActor* Soil);

	/**
	 * Handler for water level changed delegate.
	 */
	UFUNCTION()
	void OnWaterLevelChanged(AActor* Soil, float NewWaterLevel);

	/**
	 * Handler for soil state changed delegate.
	 */
	UFUNCTION()
	void OnSoilStateChanged(AActor* Soil, ESoilState NewState);

	/**
	 * Spawn a crop actor on this soil plot.
	 * @param CropData The crop data asset to spawn
	 * @return The spawned crop actor, or nullptr if failed
	 */
	ACropBase* SpawnCrop(UCropDataAsset* CropData);

	/**
	 * Internal method to initialize soil data.
	 * @param InSoilData The soil data asset to use for configuration, or nullptr for empty plot
	 */
	void InitializeSoil(USoilDataAsset* InSoilData);

	/** Soil component managing state and water */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USoilComponent> SoilComponent;

	/** Visual representation of the container */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ContainerMeshComponent;

	/** Visual representation of the soil */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SoilMeshComponent;

	/** Attachment point for crops */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> CropSpawnPoint;

	/** Dynamic material instance for the soil mesh */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicSoilMaterial;

	/** Configuration data for the container */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Plot")
	TObjectPtr<USoilContainerDataAsset> ContainerDataAsset;

	/** Configuration data for this soil */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Plot")
	TObjectPtr<USoilDataAsset> SoilDataAsset;

public:
	TObjectPtr<USoilDataAsset> GetSoilDataAsset() const
	{
		return SoilDataAsset;
	}

	TObjectPtr<USoilContainerDataAsset> GetContainerDataAsset() const
	{
		return ContainerDataAsset;
	}

protected:
	/** Class of crop actor to spawn when planting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Plot")
	TSubclassOf<ACropBase> CropActorClass;
};