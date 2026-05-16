#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../ENUM/EFarmerRole.h"
#include "../ENUM/ESkillNodeUnlockType.h"
#include "USkillTreeDataAsset.generated.h"

class USkillNodeDataAsset;

/**
 * Container for the entire player skill tree.
 * Create one DA_SkillTree asset and reference all USkillNodeDataAsset instances here.
 * Assign to USkillTreeComponent::SkillTree on the player character Blueprint.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API USkillTreeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Tree")
	TArray<TObjectPtr<USkillNodeDataAsset>> Nodes;

	/** Returns all nodes of the given unlock type. */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<USkillNodeDataAsset*> GetNodesByType(ESkillNodeUnlockType Type) const;

	/** Returns the node that unlocks a specific villager role, or nullptr if none exists. */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	USkillNodeDataAsset* FindRoleNode(EFarmerRole Role) const;
};
