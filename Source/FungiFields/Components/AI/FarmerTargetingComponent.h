#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FarmerTargetingComponent.generated.h"

class UFarmingComponent;
class USeedDataAsset;
enum class EToolType : uint8;

/**
 * Reusable AI target acquisition component for villager farming roles.
 * Harvester queries UCropManagerSubsystem; Planter and Waterer query USoilManagerSubsystem.
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
	AActor* FindBestPlantTarget(const FVector& Origin, USeedDataAsset* AvailableSeed);

	UFUNCTION(BlueprintCallable, Category = "AI|Targeting")
	AActor* FindBestWaterTarget(const FVector& Origin);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Targeting", meta = (ClampMin = "100.0"))
	float HarvestScanRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Targeting", meta = (ClampMin = "0.05"))
	float HarvestScanInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Targeting", meta = (ClampMin = "100.0"))
	float PlantScanRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Targeting", meta = (ClampMin = "100.0"))
	float WaterScanRadius = 2500.0f;

private:
	UPROPERTY()
	TObjectPtr<AActor> CachedHarvestTarget;

	float LastHarvestScanTimestamp = -1000.0f;

	bool ShouldRescanHarvestTargets(float CurrentTimeSeconds) const;
	bool IsActorValidHarvestTarget(AActor* Candidate, const FVector& Origin, const UFarmingComponent* FarmingComponent, EToolType ToolType, float& OutDistanceSquared) const;
};
