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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool InteractTool(EToolType ToolType, AActor* Interactor, float ToolPower);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool CanAcceptSeed() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool PlantSeed(UCropDataAsset* CropToPlant, AActor* Planter);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FText GetInteractionText() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool CanInteractWithTool(EToolType ToolType, AActor* Interactor) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FVector GetActionLocation() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	float GetInteractionRange() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool CanAcceptSoilBag(class UItemDataAsset* SoilBagItem) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	bool AddSoilFromBag(class UItemDataAsset* SoilBagItem);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FText GetCannotPlantReason() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FText GetCannotUseToolReason(EToolType ToolType) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Farming")
	FText GetCannotAcceptSoilBagReason() const;
};

