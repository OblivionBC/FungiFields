#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FarmerTargetingComponent.generated.h"

class UFarmingComponent;
enum class EToolType : uint8;

/**
 * Reusable AI target acquisition component for villager farming roles.
 * Phase 1 provides harvest targeting; future roles reuse the same query pattern.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UFarmerTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFarmerTargetingComponent();

	UFUNCTION(BlueprintCallable, Category = "AI|Targeting")
	AActor* FindBestHarvestTarget(const FVector& Origin, const UFarmingComponent* FarmingComponent, EToolType ToolType);

	UFUNCTION(BlueprintCallable, Category = "AI|Targeting")
	AActor* FindBestPlantTarget(const FVector& Origin);

	UFUNCTION(BlueprintCallable, Category = "AI|Targeting")
	AActor* FindBestWaterTarget(const FVector& Origin);

protected:
	/** Max radius for periodic target scans to keep workload bounded. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Targeting", meta = (ClampMin = "100.0"))
	float HarvestScanRadius = 2500.0f;

	/** Minimum interval between expensive full harvest scans. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Targeting", meta = (ClampMin = "0.05"))
	float HarvestScanInterval = 0.5f;

private:
	UPROPERTY()
	TObjectPtr<AActor> CachedHarvestTarget;

	float LastHarvestScanTimestamp = -1000.0f;

	bool ShouldRescanHarvestTargets(float CurrentTimeSeconds) const;
	bool IsActorValidHarvestTarget(AActor* Candidate, const FVector& Origin, const UFarmingComponent* FarmingComponent, EToolType ToolType, float& OutDistanceSquared) const;
};
