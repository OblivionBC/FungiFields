#include "USkillTreeDataAsset.h"
#include "USkillNodeDataAsset.h"

TArray<USkillNodeDataAsset*> USkillTreeDataAsset::GetNodesByType(ESkillNodeUnlockType Type) const
{
	TArray<USkillNodeDataAsset*> Result;
	for (const TObjectPtr<USkillNodeDataAsset>& Node : Nodes)
	{
		if (IsValid(Node) && Node->UnlockType == Type)
			Result.Add(Node.Get());
	}
	return Result;
}

USkillNodeDataAsset* USkillTreeDataAsset::FindRoleNode(EFarmerRole Role) const
{
	for (const TObjectPtr<USkillNodeDataAsset>& Node : Nodes)
	{
		if (IsValid(Node) && Node->UnlockType == ESkillNodeUnlockType::VillagerRole && Node->VillagerRole == Role)
			return Node.Get();
	}
	return nullptr;
}
