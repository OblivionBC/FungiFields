#pragma once

#include "CoreMinimal.h"
#include "EQuestEventType.generated.h"

/**
 * Types of events that quests can listen to.
 * Used to determine which game events should trigger quest progress.
 */
UENUM(BlueprintType)
enum class EQuestEventType : uint8
{
	None				UMETA(DisplayName = "None"),
	ItemAdded			UMETA(DisplayName = "Item Added"),
	ItemRemoved			UMETA(DisplayName = "Item Removed"),
	CropHarvested		UMETA(DisplayName = "Crop Harvested"),
	SeedPlanted			UMETA(DisplayName = "Seed Planted"),
	SoilTilled			UMETA(DisplayName = "Soil Tilled"),
	SoilWatered			UMETA(DisplayName = "Soil Watered"),
	CropFullyGrown		UMETA(DisplayName = "Crop Fully Grown"),
	CropWithered		UMETA(DisplayName = "Crop Withered"),
	FarmingActionPerformed UMETA(DisplayName = "Farming Action Performed")
};

