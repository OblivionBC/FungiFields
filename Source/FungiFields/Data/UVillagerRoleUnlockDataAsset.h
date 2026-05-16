#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../ENUM/EFarmerRole.h"
#include "UVillagerRoleUnlockDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FUNGIFIELDS_API FVillagerRoleUnlockEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Role Unlock")
	EFarmerRole Role = EFarmerRole::Harvester;

	/** Player level at which this role becomes growable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Role Unlock", meta = (ClampMin = 1))
	int32 RequiredLevel = 1;
};

/**
 * Data asset defining which player levels unlock each villager role.
 * Assign to LevelComponent::RoleUnlockData on the player Blueprint.
 *
 * Editor setup: create DA_VillagerRoleUnlocks and populate RoleUnlocks,
 * then assign to the player character's LevelComponent.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API UVillagerRoleUnlockDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Role Unlocks")
	TArray<FVillagerRoleUnlockEntry> RoleUnlocks;

	UFUNCTION(BlueprintPure, Category = "Role Unlocks")
	bool IsRoleUnlocked(EFarmerRole Role, int32 CurrentLevel) const;

	/** Returns all roles whose RequiredLevel exactly equals Level — used by LevelComponent to fire per-level unlock events. */
	TArray<EFarmerRole> GetRolesUnlockedAtLevel(int32 Level) const;
};
