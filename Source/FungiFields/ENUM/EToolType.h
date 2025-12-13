#pragma once

#include "EToolType.generated.h"

/**
 * Enum defining the types of farming tools available.
 */
UENUM(BlueprintType)
enum class EToolType : uint8
{
	None = 0,
	Hoe,
	WateringCan,
	Scythe
};