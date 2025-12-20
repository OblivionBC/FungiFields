#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UCropManagerSubsystem.generated.h"

class UCropGrowthComponent;

/**
 * Centralized manager for all crop growth in the world.
 * Uses a single timer to update all crops, improving performance.
 * Follows the Manager Pattern (UWorldSubsystem) as per design guidelines.
 */
UCLASS()
class FUNGIFIELDS_API UCropManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Register a crop growth component to be updated by the manager.
	 * @param GrowthComponent The growth component to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void RegisterCrop(UCropGrowthComponent* GrowthComponent);

	/**
	 * Unregister a crop growth component from the manager.
	 * @param GrowthComponent The growth component to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void UnregisterCrop(UCropGrowthComponent* GrowthComponent);

	/**
	 * Pause all crop growth.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void PauseAllGrowth();

	/**
	 * Resume all crop growth.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void ResumeAllGrowth();

	/**
	 * Get the number of registered crops.
	 * @return Number of active crops
	 */
	UFUNCTION(BlueprintPure, Category = "Crop Manager")
	int32 GetRegisteredCropCount() const { return RegisteredCrops.Num(); }

protected:
	/**
	 * Timer callback that updates all registered crops.
	 */
	UFUNCTION()
	void OnGrowthUpdateTimer();

private:
	/** Set of all registered crop growth components */
	UPROPERTY()
	TSet<TObjectPtr<UCropGrowthComponent>> RegisteredCrops;

	/** Timer handle for the global growth update */
	FTimerHandle GrowthUpdateTimerHandle;

	/** Interval for growth updates (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Crop Manager Settings", meta = (ClampMin = "0.1"))
	float GrowthUpdateInterval = 1.0f;

	/** Whether growth is currently paused */
	bool bGrowthPaused = false;
};

