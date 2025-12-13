#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../Data/FHarvestResult.h"
#include "ITooltipProvider.h"
#include "IHarvestableInterface.generated.h"

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
	/**
	 * Harvest this actor.
	 * @param Harvester The actor performing the harvest
	 * @return Struct containing the harvest result (item and quantity)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FHarvestResult Harvest(AActor* Harvester, float ToolPower);

	/**
	 * Check if this actor can be harvested.
	 * @return True if ready to harvest
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool CanHarvest() const;

	/**
	 * Get contextual interaction text for harvesting.
	 * @return Text to display for harvest interaction
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FText GetHarvestText() const;

	/**
	 * Get the world location where harvesting should be performed.
	 * Useful for AI pathfinding and positioning.
	 * @return World location for harvest action
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FVector GetActionLocation() const;

	/**
	 * Get the maximum interaction range for harvesting.
	 * @return Maximum distance an actor can be to harvest
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	float GetInteractionRange() const;
};