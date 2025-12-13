#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../ENUM/EToolType.h"
#include "ITooltipProvider.h"
#include "IFarmableInterface.generated.h"

class UCropDataAsset;

/**
 * Interface for actors that can be farmed (soil plots, etc.).
 * Allows decoupled interaction with farming tools without direct casting.
 */
UINTERFACE(MinimalAPI)
class UFarmableInterface : public UInterface
{
	GENERATED_BODY()
};

class IFarmableInterface : public ITooltipProvider
{
	GENERATED_BODY()

public:
	/**
	 * Interact with this farmable using a tool.
	 * @param ToolType The type of tool being used
	 * @param Interactor The actor using the tool
	 * @param ToolPower The power of the tool actor is using
	 * @return True if the interaction was successful
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool InteractTool(EToolType ToolType, AActor* Interactor, float ToolPower);

	/**
	 * Check if this farmable can accept a seed.
	 * @return True if a seed can be planted
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool CanAcceptSeed() const;

	/**
	 * Plant a seed on this farmable.
	 * @param CropToPlant The crop data asset to plant
	 * @param Planter The actor planting the seed
	 * @return True if planting was successful
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool PlantSeed(UCropDataAsset* CropToPlant, AActor* Planter);

	/**
	 * Get contextual interaction text for this farmable.
	 * @return Text to display for interaction
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FText GetInteractionText() const;

	/**
	 * Check if a tool can interact with this farmable.
	 * @param ToolType The type of tool being used
	 * @param Interactor The actor using the tool
	 * @return True if the tool can perform an action on this farmable
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool CanInteractWithTool(EToolType ToolType, AActor* Interactor) const;

	/**
	 * Get the world location where farming actions should be performed.
	 * Useful for AI pathfinding and positioning.
	 * @return World location for action execution
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FVector GetActionLocation() const;

	/**
	 * Get the maximum interaction range for this farmable.
	 * @return Maximum distance an actor can be to interact
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	float GetInteractionRange() const;
};

