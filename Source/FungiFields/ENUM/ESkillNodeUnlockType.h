#pragma once

#include "ESkillNodeUnlockType.generated.h"

UENUM(BlueprintType)
enum class ESkillNodeUnlockType : uint8
{
	VillagerRole   UMETA(DisplayName = "Villager Role"),    // unlocks growing a villager of this role
	ItemSchematic  UMETA(DisplayName = "Item Schematic"),   // unlocks crafting/purchasing an item
	Building       UMETA(DisplayName = "Building"),         // unlocks placing a building type
	PassiveStat    UMETA(DisplayName = "Passive Stat"),     // grants a tagged stat bonus
};
