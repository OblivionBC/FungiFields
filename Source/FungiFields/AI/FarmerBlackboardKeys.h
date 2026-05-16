#pragma once

#include "CoreMinimal.h"

/**
 * Canonical Blackboard key names for the farmer villager Behavior Tree.
 *
 * When creating DA_FarmerBlackboard in the editor, add these keys with the types listed:
 *   TargetActor    — Object (base class: Actor)
 *   WanderTarget   — Vector
 *   HomeLocation   — Vector
 *   bIsStarving    — Bool
 *   bIsHungry      — Bool
 *   WorkSpeed      — Float
 *
 * All BT tasks and services must reference keys via these constants, not raw string literals.
 */
namespace FarmerBBKeys
{
	/** The farming target currently assigned (harvest/plant/water). Written by BTTask_FindFarmTarget. */
	static const FName TargetActor    (TEXT("TargetActor"));

	/** Random nav point to wander toward. Written by BTTask_VillagerWander. */
	static const FName WanderTarget   (TEXT("WanderTarget"));

	/** Villager's spawn location — set once by BTService_UpdateVillagerNeeds. */
	static const FName HomeLocation   (TEXT("HomeLocation"));

	/** True when hunger <= StarvingThreshold. Forces wander-only behaviour. */
	static const FName bIsStarving    (TEXT("bIsStarving"));

	/** True when hunger <= LowHungerThreshold but not yet starving. */
	static const FName bIsHungry      (TEXT("bIsHungry"));

	/** Current work speed multiplier (1.0 / 0.5 / 0.0). */
	static const FName WorkSpeed      (TEXT("WorkSpeed"));
}
