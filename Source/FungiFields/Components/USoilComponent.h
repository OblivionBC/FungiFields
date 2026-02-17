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

	/** @return Fertility multiplier (1.0 = normal, >1.0 = faster) */
	UFUNCTION(BlueprintPure, Category = "Soil")
	float GetEffectiveFertility() const;

	UFUNCTION(BlueprintPure, Category = "Soil")
	float GetWaterLevel() const { return CurrentWaterLevel; }

	UFUNCTION(BlueprintPure, Category = "Soil")
	bool HasWater() const { return CurrentWaterLevel > 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Soil")
	void AddWater(float Amount);

	/** @return True if water was available and consumed */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	bool ConsumeWater(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Soil")
	void SetCrop(ACropBase* Crop);

	UFUNCTION(BlueprintPure, Category = "Soil")
	ACropBase* GetCrop() const { return HeldCrop; }

	UFUNCTION(BlueprintPure, Category = "Soil")
	USoilDataAsset* GetSoilData() const { return SoilData; }

	/** @return True if tilling was successful */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	bool Till(const float TillPower);

	UFUNCTION(BlueprintPure, Category = "Soil")
	bool IsTilled() const { return bIsTilled; }

	/** @return True if tilled and no crop is present */
	UFUNCTION(BlueprintPure, Category = "Soil")
	bool CanAcceptCrop() const;

	UFUNCTION(BlueprintCallable, Category = "Soil")
	void RemoveCrop();

	UFUNCTION(BlueprintPure, Category = "Soil")
	bool HasSoil() const { return SoilData != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Soil")
	void SetSoilType(USoilDataAsset* InSoilData);

	/** @return Empty if no soil, Dry if soil present but not watered, Wet if soil present and watered */
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
	UFUNCTION()
	void OnWaterEvaporationTimer();

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

