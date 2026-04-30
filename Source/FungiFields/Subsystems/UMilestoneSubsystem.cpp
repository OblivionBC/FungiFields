#include "UMilestoneSubsystem.h"
#include "../Data/UMilestoneDataAsset.h"
#include "../Data/USporeJournalDataAsset.h"

void UMilestoneSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMilestoneSubsystem::Deinitialize()
{
	MilestoneCounts.Empty();
	MilestoneDefinitions.Empty();
	Super::Deinitialize();
}

void UMilestoneSubsystem::RegisterMilestoneData(USporeJournalDataAsset* JournalData)
{
	if (!JournalData)
		return;

	for (UMilestoneDataAsset* Milestone : JournalData->Milestones)
	{
		if (IsValid(Milestone) && !Milestone->MilestoneID.IsNone())
		{
			MilestoneDefinitions.FindOrAdd(Milestone->MilestoneID) = Milestone;
		}
	}
}

void UMilestoneSubsystem::ReportEvent(FName MilestoneID, int32 Amount)
{
	if (MilestoneID.IsNone() || Amount <= 0)
		return;

	int32& Count = MilestoneCounts.FindOrAdd(MilestoneID, 0);
	const int32 OldCount = Count;
	Count += Amount;

	CheckTierCrossing(MilestoneID, OldCount, Count);
}

int32 UMilestoneSubsystem::GetProgress(FName MilestoneID) const
{
	if (const int32* Found = MilestoneCounts.Find(MilestoneID))
	{
		return *Found;
	}
	return 0;
}

void UMilestoneSubsystem::CheckTierCrossing(FName MilestoneID, int32 OldCount, int32 NewCount)
{
	const TObjectPtr<UMilestoneDataAsset>* FoundDef = MilestoneDefinitions.Find(MilestoneID);
	if (!FoundDef || !IsValid(*FoundDef))
		return;

	const UMilestoneDataAsset* Def = FoundDef->Get();
	if (Def->TierThresholds.IsEmpty())
		return;

	for (int32 TierIndex = 0; TierIndex < Def->TierThresholds.Num(); ++TierIndex)
	{
		const int32 Threshold = Def->TierThresholds[TierIndex];
		if (OldCount < Threshold && NewCount >= Threshold)
		{
			OnMilestoneReached.Broadcast(MilestoneID, TierIndex);
		}
	}
}
