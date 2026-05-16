#include "USkillTreeComponent.h"
#include "LevelComponent.h"
#include "../Data/USkillTreeDataAsset.h"
#include "../Data/USkillNodeDataAsset.h"
#include "../Data/UItemDataAsset.h"

USkillTreeComponent::USkillTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ULevelComponent* LevelComp = GetOwner()->FindComponentByClass<ULevelComponent>())
		LevelComp->OnLevelUp.AddDynamic(this, &USkillTreeComponent::OnOwnerLevelUp);
}

// ── Queries ──────────────────────────────────────────────────────────────────

bool USkillTreeComponent::IsNodeUnlocked(const USkillNodeDataAsset* Node) const
{
	return IsValid(Node) && UnlockedNodeIDs.Contains(Node->NodeID);
}

bool USkillTreeComponent::CanUnlockNode(const USkillNodeDataAsset* Node) const
{
	if (!IsValid(Node) || IsNodeUnlocked(Node))
		return false;
	if (AvailableSkillPoints < Node->PointCost)
		return false;
	return Node->ArePrerequisitesMet(UnlockedNodeIDs);
}

bool USkillTreeComponent::IsRoleUnlocked(EFarmerRole Role) const
{
	if (!SkillTree) return false;
	for (const TObjectPtr<USkillNodeDataAsset>& Node : SkillTree->Nodes)
	{
		if (IsValid(Node) && Node->UnlockType == ESkillNodeUnlockType::VillagerRole
			&& Node->VillagerRole == Role && UnlockedNodeIDs.Contains(Node->NodeID))
			return true;
	}
	return false;
}

bool USkillTreeComponent::IsSchematicUnlocked(const UItemDataAsset* Item) const
{
	if (!SkillTree || !Item) return false;
	const FSoftObjectPath ItemPath(Item);
	for (const TObjectPtr<USkillNodeDataAsset>& Node : SkillTree->Nodes)
	{
		if (IsValid(Node) && Node->UnlockType == ESkillNodeUnlockType::ItemSchematic
			&& Node->ItemSchematic.ToSoftObjectPath() == ItemPath && UnlockedNodeIDs.Contains(Node->NodeID))
			return true;
	}
	return false;
}

bool USkillTreeComponent::IsBuildingUnlocked(FName BuildingTag) const
{
	if (!SkillTree) return false;
	for (const TObjectPtr<USkillNodeDataAsset>& Node : SkillTree->Nodes)
	{
		if (IsValid(Node) && Node->UnlockType == ESkillNodeUnlockType::Building
			&& Node->BuildingTag == BuildingTag && UnlockedNodeIDs.Contains(Node->NodeID))
			return true;
	}
	return false;
}

// ── Mutations ────────────────────────────────────────────────────────────────

bool USkillTreeComponent::TryUnlockNode(USkillNodeDataAsset* Node)
{
	if (!CanUnlockNode(Node))
		return false;

	AvailableSkillPoints -= Node->PointCost;
	UnlockedNodeIDs.Add(Node->NodeID);

	OnSkillPointsChanged.Broadcast(AvailableSkillPoints);
	OnSkillNodeUnlocked.Broadcast(Node);

	return true;
}

void USkillTreeComponent::AddSkillPoints(int32 Amount)
{
	if (Amount <= 0) return;
	AvailableSkillPoints += Amount;
	OnSkillPointsChanged.Broadcast(AvailableSkillPoints);
}

// ── Save / Load ──────────────────────────────────────────────────────────────

void USkillTreeComponent::LoadUnlockedNodeIDs(const TArray<FName>& SavedIDs)
{
	UnlockedNodeIDs = SavedIDs;
}

void USkillTreeComponent::LoadSkillPoints(int32 SavedPoints)
{
	AvailableSkillPoints = FMath::Max(0, SavedPoints);
	OnSkillPointsChanged.Broadcast(AvailableSkillPoints);
}

// ── Private ──────────────────────────────────────────────────────────────────

void USkillTreeComponent::OnOwnerLevelUp(int32 NewLevel)
{
	AddSkillPoints(1);
}
