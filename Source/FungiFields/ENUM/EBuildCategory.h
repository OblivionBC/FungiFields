#pragma once

#include "EBuildCategory.generated.h"

UENUM(BlueprintType)
enum class EBuildCategory : uint8
{
	None		UMETA(DisplayName = "None"),
	FarmPlot	UMETA(DisplayName = "Farm Plot"),
	Decoration	UMETA(DisplayName = "Decoration"),
	Structure	UMETA(DisplayName = "Structure")
};
