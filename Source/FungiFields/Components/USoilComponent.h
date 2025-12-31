#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../ENUM/ESoilState.h"
#include "USoilComponent.generated.h"

class USoilDataAsset;
class ACropBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoilTilledState, AActor*, Soil);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCropPlanted, AActor*, Soil, ACropBase*, Crop);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCropRemoved, AActor*, Soil);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaterLevelChanged, AActor*, Soil, float, NewWaterLevel);

/**
 * Component responsible for managing soil state and water level.
 * Handles tilling, crop placement, and water management with timer-based evaporation.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API USoilComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USoilComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Initialize the soil component with soil data asset.
	 * @param InSoilData The soil data asset to use for configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	void Initialize(USoilDataAsset* InSoilData);

	/**
	 * Get the effective fertility rate for crop growth.
	 * @return Fertility multiplier (1.0 = normal, >1.0 = faster)
	 */
	UFUNCTION(BlueprintPure, Category = "Soil")
	float GetEffectiveFertility() const;

	/**
	 * Get the current water level.
	 * @return Current water level (0.0 to MaxWaterLevel)
	 */
	UFUNCTION(BlueprintPure, Category = "Soil")
	float GetWaterLevel() const { return CurrentWaterLevel; }

	/**
	 * Check if water is available for crop growth.
	 * @return True if water level > 0
	 */
	UFUNCTION(BlueprintPure, Category = "Soil")
	bool HasWater() const { return CurrentWaterLevel > 0.0f; }

	/**
	 * Add water to the soil.
	 * @param Amount Amount of water to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	void AddWater(float Amount);

	/**
	 * Consume water from the soil (used by crops).
	 * @param Amount Amount of water to consume
	 * @return True if water was available and consumed
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	bool ConsumeWater(float Amount);

	/**
	 * Set the crop on this soil.
	 * @param Crop The crop actor to place on this soil
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	void SetCrop(ACropBase* Crop);

	/**
	 * Get the current crop on this soil.
	 * @return The crop actor, or nullptr if no crop
	 */
	UFUNCTION(BlueprintPure, Category = "Soil")
	ACropBase* GetCrop() const { return HeldCrop; }

	/**
	 * Get the soil data asset.
	 * @return The soil data asset, or nullptr if not initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Soil")
	USoilDataAsset* GetSoilData() const { return SoilData; }

	/**
	 * Till the soil (make it ready for planting).
	 * @return True if tilling was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	bool Till(const float TillPower);

	/**
	 * Check if the soil is tilled.
	 * @return True if soil is tilled
	 */
	UFUNCTION(BlueprintPure, Category = "Soil")
	bool IsTilled() const { return bIsTilled; }

	/**
	 * Check if the soil can accept a crop.
	 * @return True if tilled and no crop is present
	 */
	UFUNCTION(BlueprintPure, Category = "Soil")
	bool CanAcceptCrop() const;

	/**
	 * Remove the current crop from this soil.
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	void RemoveCrop();

	/**
	 * Check if soil is present in the plot.
	 * @return True if soil data asset is set
	 */
	UFUNCTION(BlueprintPure, Category = "Soil")
	bool HasSoil() const { return SoilData != nullptr; }

	/**
	 * Set the soil type for this plot.
	 * @param InSoilData The soil data asset to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	void SetSoilType(USoilDataAsset* InSoilData);

	/**
	 * Get the current soil state.
	 * @return Empty if no soil, Dry if soil present but not watered, Wet if soil present and watered
	 */
	UFUNCTION(BlueprintPure, Category = "Soil")
	ESoilState GetSoilState() const;

	/** Delegate broadcast when soil is tilled */
	UPROPERTY(BlueprintAssignable, Category = "Soil")
	FOnSoilTilledState OnSoilTilled;

	/** Delegate broadcast when a crop is planted */
	UPROPERTY(BlueprintAssignable, Category = "Soil")
	FOnCropPlanted OnCropPlanted;

	/** Delegate broadcast when a crop is removed */
	UPROPERTY(BlueprintAssignable, Category = "Soil")
	FOnCropRemoved OnCropRemoved;

	/** Delegate broadcast when water level changes */
	UPROPERTY(BlueprintAssignable, Category = "Soil")
	FOnWaterLevelChanged OnWaterLevelChanged;

	/** Delegate broadcast when soil state changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSoilStateChanged, AActor*, Soil, ESoilState, NewState);
	UPROPERTY(BlueprintAssignable, Category = "Soil")
	FOnSoilStateChanged OnSoilStateChanged;

protected:
	/**
	 * Timer callback for water evaporation.
	 * Decreases water level over time based on retention multiplier.
	 */
	UFUNCTION()
	void OnWaterEvaporationTimer();

	/**
	 * Update visual representation based on water level.
	 * Called when water level changes (event-driven, not Tick).
	 */
	void UpdateVisuals();

	/** Current Tills to Progress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Plot")
	float TillProgress;

	/** Tills needed to change to tilled soil */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Plot")
	float TillThreshold = 0.0f;

private:
	/** Configuration data for this soil */
	UPROPERTY(EditAnywhere, Category = "Soil Data")
	TObjectPtr<USoilDataAsset> SoilData;

	/** Current water level */
	UPROPERTY(VisibleAnywhere, Category = "Soil Data")
	float CurrentWaterLevel = 0.0f;

	/** Reference to the crop on this soil */
	UPROPERTY(VisibleAnywhere, Category = "Soil Data")
	TObjectPtr<ACropBase> HeldCrop = nullptr;

	/** Whether the soil has been tilled */
	UPROPERTY(VisibleAnywhere, Category = "Soil Data")
	bool bIsTilled = false;

	/** Timer handle for water evaporation */
	FTimerHandle WaterEvaporationTimerHandle;

	/** Rate at which water evaporates (units per second) */
	UPROPERTY(EditDefaultsOnly, Category = "Soil Settings", meta = (ClampMin = "0.0"))
	float WaterEvaporationRate = 1.0f;

	/** Interval for water evaporation timer (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Soil Settings", meta = (ClampMin = "0.1"))
	float EvaporationCheckInterval = 1.0f;
};

