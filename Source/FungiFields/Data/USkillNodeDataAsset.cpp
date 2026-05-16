#include "USkillNodeDataAsset.h"

bool USkillNodeDataAsset::ArePrerequisitesMet(const TArray<FName>& UnlockedNodeIDs) const
{
	for (const TObjectPtr<USkillNodeDataAsset>& Prereq : Prerequisites)
	{
		if (!IsValid(Prereq) || !UnlockedNodeIDs.Contains(Prereq->NodeID))
			return false;
	}
	return true;
}
