#include "UVillagerRoleUnlockDataAsset.h"

bool UVillagerRoleUnlockDataAsset::IsRoleUnlocked(EFarmerRole Role, int32 CurrentLevel) const
{
	for (const FVillagerRoleUnlockEntry& Entry : RoleUnlocks)
	{
		if (Entry.Role == Role)
			return CurrentLevel >= Entry.RequiredLevel;
	}
	return false;
}

TArray<EFarmerRole> UVillagerRoleUnlockDataAsset::GetRolesUnlockedAtLevel(int32 Level) const
{
	TArray<EFarmerRole> Result;
	for (const FVillagerRoleUnlockEntry& Entry : RoleUnlocks)
	{
		if (Entry.RequiredLevel == Level)
			Result.Add(Entry.Role);
	}
	return Result;
}
