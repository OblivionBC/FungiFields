#pragma once

#include "CoreMinimal.h"
#include "ESoilState.generated.h"

/**
 * Enum representing the state of soil in a plot.
 */
UENUM(BlueprintType)
enum class ESoilState : uint8
{
	/** No soil in the plot (empty container) */
	Empty	UMETA(DisplayName = "Empty"),
	
	/** Soil is present but not watered */
	Dry		UMETA(DisplayName = "Dry"),
	
	/** Soil is present and watered */
	Wet		UMETA(DisplayName = "Wet")
};


