#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../Data/FHarvestResult.h"
#include "ITooltipProvider.h"
#include "IHarvestableInterface.generated.h"

class UCropDataAsset;

/**
 * Interface for actors that can be harvested (crops, etc.).
 * Allows decoupled harvest interaction without direct casting.
 */
UINTERFACE(MinimalAPI)
class UHarvestableInterface : public UInterface
{
	GENERATED_BODY()
};

class IHarvestableInterface : public ITooltipProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FHarvestResult Harvest(AActor* Harvester, float ToolPower);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool CanHarvest() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FText GetHarvestText() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FVector GetActionLocation() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	float GetInteractionRange() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	UCropDataAsset* GetCropData() const;

	virtual UCropDataAsset* GetCropData_Implementation() const { return nullptr; }
};