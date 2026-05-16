#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../ENUM/EFarmerRole.h"
#include "../ENUM/ESkillNodeUnlockType.h"
#include "USkillNodeDataAsset.generated.h"

class UItemDataAsset;
class UTexture2D;

/**
 * Defines a single node in the player skill tree.
 *
 * NodeID must be unique across all nodes — it is used as the save key.
 * Prerequisites: all listed nodes must be unlocked before this one can be purchased.
 *
 * Editor setup: create one DA_SkillNode_* asset per node, then reference them
 * all from DA_SkillTree (USkillTreeDataAsset).
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API USkillNodeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Unique stable identifier — used for save/load. Never rename after shipping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node")
	FName NodeID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node", meta = (ClampMin = 1))
	int32 PointCost = 1;

	/** All of these nodes must be unlocked before this one can be purchased. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node")
	TArray<TObjectPtr<USkillNodeDataAsset>> Prerequisites;

	// ── Unlock payload ─────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unlock")
	ESkillNodeUnlockType UnlockType = ESkillNodeUnlockType::VillagerRole;

	/** Which villager role this node unlocks (seed + growing). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unlock",
		meta = (EditCondition = "UnlockType == ESkillNodeUnlockType::VillagerRole", EditConditionHides))
	EFarmerRole VillagerRole = EFarmerRole::Harvester;

	/** Item whose schematic (crafting recipe / shop listing) is unlocked. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unlock",
		meta = (EditCondition = "UnlockType == ESkillNodeUnlockType::ItemSchematic", EditConditionHides))
	TSoftObjectPtr<UItemDataAsset> ItemSchematic;

	/**
	 * Tag identifying the building type unlocked.
	 * Matched against the building actor's GameplayTag at placement time.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unlock",
		meta = (EditCondition = "UnlockType == ESkillNodeUnlockType::Building", EditConditionHides))
	FName BuildingTag;

	/**
	 * Tag identifying which stat is boosted (e.g. "Stat.HarvestYield").
	 * Resolved by whatever system consumes passive stats (GAS effect, subsystem, etc.).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unlock",
		meta = (EditCondition = "UnlockType == ESkillNodeUnlockType::PassiveStat", EditConditionHides))
	FName StatTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unlock",
		meta = (EditCondition = "UnlockType == ESkillNodeUnlockType::PassiveStat", EditConditionHides, ClampMin = 0.0f))
	float StatMagnitude = 0.f;

	// ── Helpers ─────────────────────────────────────────────────────────────────

	/** Returns true if every prerequisite NodeID appears in UnlockedNodeIDs. */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool ArePrerequisitesMet(const TArray<FName>& UnlockedNodeIDs) const;
};
