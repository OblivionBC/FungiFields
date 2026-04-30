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

	UFUNCTION(BlueprintCallable, Category = "Crop Growth")
	void Initialize(UCropDataAsset* InCropData, ASoilPlot* InParentSoil);

	UFUNCTION(BlueprintPure, Category = "Crop Growth")
	float GetGrowthProgress() const { return CurrentGrowthProgress; }

	UFUNCTION(BlueprintPure, Category = "Crop Growth")
	bool IsFullyGrown() const { return CurrentGrowthProgress >= 1.0f; }

	UFUNCTION(BlueprintPure, Category = "Crop Growth")
	bool IsWithered() const { return bIsWithered; }

	UFUNCTION(BlueprintPure, Category = "Crop Growth")
	UCropDataAsset* GetCropData() const { return CropData; }

	UFUNCTION(BlueprintCallable, Category = "Crop Growth")
	void StartGrowth();

	UFUNCTION(BlueprintCallable, Category = "Crop Growth")
	void PauseGrowth();

	void UpdateGrowth(float DeltaTime);

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