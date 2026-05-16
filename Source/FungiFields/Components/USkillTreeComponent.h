#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../ENUM/EFarmerRole.h"
#include "USkillTreeComponent.generated.h"

class USkillTreeDataAsset;
class USkillNodeDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillPointsChanged, int32, NewTotal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillNodeUnlocked,  USkillNodeDataAsset*, Node);

/**
 * Tracks available skill points and which tree nodes the player has unlocked.
 *
 * Auto-wires to ULevelComponent::OnLevelUp in BeginPlay — place both components on
 * the player character and this component will receive 1 point per level automatically.
 *
 * UnlockedNodeIDs (array of FName) is the save-game payload — serialize this array.
 *
 * Editor setup:
 *   1. Create DA_SkillTree (USkillTreeDataAsset) and populate its Nodes array.
 *   2. Assign DA_SkillTree to SkillTree on this component's Blueprint CDO.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FUNGIFIELDS_API USkillTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillTreeComponent();

	virtual void BeginPlay() override;

	// ── Setup ───────────────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Tree")
	TObjectPtr<USkillTreeDataAsset> SkillTree;

	// ── Delegates ───────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Skill Tree")
	FOnSkillPointsChanged OnSkillPointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Skill Tree")
	FOnSkillNodeUnlocked OnSkillNodeUnlocked;

	// ── Queries ─────────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	int32 GetAvailableSkillPoints() const { return AvailableSkillPoints; }

	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool IsNodeUnlocked(const USkillNodeDataAsset* Node) const;

	/** Returns true if the node exists, prerequisites are met, and cost can be afforded. */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool CanUnlockNode(const USkillNodeDataAsset* Node) const;

	/** Checks whether any VillagerRole node for this role has been unlocked. */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool IsRoleUnlocked(EFarmerRole Role) const;

	/** Checks whether any ItemSchematic node for the given item has been unlocked. */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool IsSchematicUnlocked(const UItemDataAsset* Item) const;

	/** Checks whether the building tag has been unlocked via any Building node. */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool IsBuildingUnlocked(FName BuildingTag) const;

	// ── Mutations ───────────────────────────────────────────────────────────────

	/**
	 * Attempts to unlock the node. Returns false if prerequisites aren't met or
	 * the player can't afford it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree")
	bool TryUnlockNode(USkillNodeDataAsset* Node);

	/** Adds skill points directly (called automatically on level-up, or by cheats/events). */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree")
	void AddSkillPoints(int32 Amount);

	// ── Save / Load ─────────────────────────────────────────────────────────────

	/** Returns the raw unlocked ID list for serialization. */
	UFUNCTION(BlueprintPure, Category = "Skill Tree|Save")
	const TArray<FName>& GetUnlockedNodeIDs() const { return UnlockedNodeIDs; }

	/** Restores unlocked node state from a previously saved ID list. */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree|Save")
	void LoadUnlockedNodeIDs(const TArray<FName>& SavedIDs);

	/** Restores skill point total (call alongside LoadUnlockedNodeIDs on game load). */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree|Save")
	void LoadSkillPoints(int32 SavedPoints);

private:
	UPROPERTY(VisibleAnywhere, Category = "Skill Tree")
	int32 AvailableSkillPoints = 0;

	UPROPERTY(VisibleAnywhere, Category = "Skill Tree")
	TArray<FName> UnlockedNodeIDs;

	UFUNCTION()
	void OnOwnerLevelUp(int32 NewLevel);
};
