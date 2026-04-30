#pragma once

#include "EFarmerJobState.generated.h"

UENUM()
enum class EFarmerJobState : uint8
{
	Idle,
	AcquiringTarget,
	MovingToTarget,
	PerformingAction,
	Cooldown,
	Wandering
};
