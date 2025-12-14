#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UCropGrowthComponent.generated.h"

class UCropDataAsset;
class ASoilPlot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrowthStageChanged, AActor*, Crop, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCropFullyGrown, AActor*, Crop);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCropWithered, AActor*, Crop);

/**
 * Component responsible for managing crop growth lifecycle.
 * Uses timer-based growth instead of Tick for performance.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UCropGrowthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCropGrowthComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Initialize the growth component with crop data and parent soil.
	 * @param InCropData The crop data asset to use for configuration
	 * @param InParentSoil The soil plot this crop is planted on
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Growth")
	void Initialize(UCropDataAsset* InCropData, ASoilPlot* InParentSoil);

	/**
	 * Get the current growth progress (0.0 to 1.0).
	 * @return Growth progress as a float
	 */
	UFUNCTION(BlueprintPure, Category = "Crop Growth")
	float GetGrowthProgress() const { return CurrentGrowthProgress; }

	/**
	 * Check if the crop is fully grown.
	 * @return True if growth progress >= 1.0
	 */
	UFUNCTION(BlueprintPure, Category = "Crop Growth")
	bool IsFullyGrown() const { return CurrentGrowthProgress >= 1.0f; }

	/**
	 * Check if the crop has withered.
	 * @return True if crop has died from lack of water
	 */
	UFUNCTION(BlueprintPure, Category = "Crop Growth")
	bool IsWithered() const { return bIsWithered; }

	/**
	 * Start the growth (registers with crop manager).
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Growth")
	void StartGrowth();

	/**
	 * Pause the growth (unregisters from crop manager).
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Growth")
	void PauseGrowth();

	/**
	 * Update growth progress. Called by UCropManagerSubsystem.
	 * @param DeltaTime Time since last update
	 */
	void UpdateGrowth(float DeltaTime);

	/**
	 * Update the crop mesh based on growth progress.
	 * Called when growth stage threshold is crossed.
	 * Public so it can be called to trigger initial mesh update.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Growth")
	void UpdateMesh();

	/** Delegate broadcast when growth stage changes (mesh update) */
	UPROPERTY(BlueprintAssignable, Category = "Crop Growth")
	FOnGrowthStageChanged OnGrowthStageChanged;

	/** Delegate broadcast when crop reaches full growth */
	UPROPERTY(BlueprintAssignable, Category = "Crop Growth")
	FOnCropFullyGrown OnCropFullyGrown;

	/** Delegate broadcast when crop withers from lack of water */
	UPROPERTY(BlueprintAssignable, Category = "Crop Growth")
	FOnCropWithered OnCropWithered;

private:
	/** Configuration data for this crop */
	UPROPERTY(VisibleAnywhere, Category = "Crop Growth Data")
	TObjectPtr<UCropDataAsset> CropData;

	/** Current growth progress (0.0 to 1.0) */
	UPROPERTY(VisibleAnywhere, Category = "Crop Growth Data")
	float CurrentGrowthProgress = 0.0f;

	/** Reference to the parent soil plot */
	UPROPERTY(VisibleAnywhere, Category = "Crop Growth Data")
	TObjectPtr<ASoilPlot> ParentSoil;

	/** Whether the crop has withered */
	UPROPERTY(VisibleAnywhere, Category = "Crop Growth Data")
	bool bIsWithered = false;

	/** Whether growth is currently active (registered with manager) */
	bool bGrowthActive = false;

	/** Time without water before crop withers (seconds) - set from crop data asset */
	float WitherTimeWithoutWater = 30.0f;

	/** Time since last water check */
	float TimeWithoutWater = 0.0f;

	/** Growth increment per second (calculated based on growth time and fertility) */
	float GrowthIncrementPerSecond = 0.01f;

	/** Last growth stage index for mesh updates */
	int32 LastGrowthStageIndex = -1;
};