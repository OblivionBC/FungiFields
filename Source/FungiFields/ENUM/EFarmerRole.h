#pragma once

#include "EFarmerRole.generated.h"

UENUM(BlueprintType)
enum class EFarmerRole : uint8
{
	Harvester UMETA(DisplayName = "Harvester"),
	Planter UMETA(DisplayName = "Planter"),
	Waterer UMETA(DisplayName = "Waterer")
};
