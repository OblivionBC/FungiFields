#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../ENUM/EToolType.h"
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

class IFarmableInterface
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
};

