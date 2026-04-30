#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UCropManagerSubsystem.generated.h"

class UCropGrowthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCropManagerCropFullyGrown, AActor*, Crop);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCropManagerCropWithered, AActor*, Crop);

UCLASS()
class FUNGIFIELDS_API UCropManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void RegisterCrop(UCropGrowthComponent* GrowthComponent);

	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void UnregisterCrop(UCropGrowthComponent* GrowthComponent);

	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void PauseAllGrowth();

	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void ResumeAllGrowth();

	/**
	 * Pauses only crops that do NOT have bGrowsAtNight set.
	 * Night-growing crops (e.g. mushrooms) continue updating.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void PauseNonNightCrops();

	/** Resumes all crops that were selectively paused by PauseNonNightCrops. */
	UFUNCTION(BlueprintCallable, Category = "Crop Manager")
	void ResumeNonNightCrops();

	UFUNCTION(BlueprintPure, Category = "Crop Manager")
	int32 GetRegisteredCropCount() const { return RegisteredCrops.Num(); }

	const TSet<TObjectPtr<UCropGrowthComponent>>& GetRegisteredCrops() const { return RegisteredCrops; }

	UPROPERTY(BlueprintAssignable, Category = "Crop Manager Events")
	FOnCropManagerCropFullyGrown OnCropFullyGrown;

	UPROPERTY(BlueprintAssignable, Category = "Crop Manager Events")
	FOnCropManagerCropWithered OnCropWithered;

protected:
	UFUNCTION()
	void OnGrowthUpdateTimer();

private:
	UFUNCTION()
	void HandleCropFullyGrown(AActor* Crop);

	UFUNCTION()
	void HandleCropWithered(AActor* Crop);

	UPROPERTY()
	TSet<TObjectPtr<UCropGrowthComponent>> RegisteredCrops;

	/** Crops temporarily excluded from growth updates during nighttime. */
	UPROPERTY()
	TSet<TObjectPtr<UCropGrowthComponent>> NightPausedCrops;

	FTimerHandle GrowthUpdateTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Crop Manager Settings", meta = (ClampMin = "0.1"))
	float GrowthUpdateInterval = 1.0f;
};
