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

	virtual FHarvestResult Harvest_Implementation(AActor* Harvester, float ToolPower) override;
	virtual bool CanHarvest_Implementation() const override;
	virtual FText GetHarvestText_Implementation() const override;
	virtual FVector GetActionLocation_Implementation() const override;
	virtual float GetInteractionRange_Implementation() const override;

	virtual FText GetTooltipText_Implementation() const override;
	virtual UCropDataAsset* GetCropData_Implementation() const override { return CropDataAsset; }

	UFUNCTION(BlueprintCallable, Category = "Crop")
	void Initialize(UCropDataAsset* InCropData, ASoilPlot* InParentSoil);

	UFUNCTION(BlueprintPure, Category = "Crop")
	UCropGrowthComponent* GetGrowthComponent() const { return GrowthComponent; }

protected:
	UFUNCTION()
	void OnGrowthStageChanged(AActor* Crop, float Progress);

	UFUNCTION()
	void OnCropFullyGrown(AActor* Crop);

	UFUNCTION()
	void OnCropWithered(AActor* Crop);

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
};