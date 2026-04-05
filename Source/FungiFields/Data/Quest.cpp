#include "Quest.h"
#include "../Data/UItemDataAsset.h"
#include "../Data/UCropDataAsset.h"
#include "../Data/USeedDataAsset.h"

bool UQuest::ShouldRespondToItemAdded(UItemDataAsset* Item, int32 Quantity) const
{
	if (QuestEventType != EQuestEventType::ItemAdded)
		return false;

	return !RequiredItem || RequiredItem == Item;
}

bool UQuest::ShouldRespondToItemRemoved(UItemDataAsset* Item, int32 Quantity) const
{
	if (QuestEventType != EQuestEventType::ItemRemoved)
		return false;

	return !RequiredItem || RequiredItem == Item;
}

bool UQuest::ShouldRespondToCropHarvested(UCropDataAsset* CropData, int32 Quantity) const
{
	if (QuestEventType != EQuestEventType::CropHarvested)
		return false;

	return !RequiredCrop || RequiredCrop == CropData;
}

bool UQuest::ShouldRespondToCropFullyGrown(UCropDataAsset* CropData) const
{
	if (QuestEventType != EQuestEventType::CropFullyGrown)
		return false;

	return !RequiredCrop || RequiredCrop == CropData;
}

bool UQuest::ShouldRespondToCropWithered(UCropDataAsset* CropData) const
{
	if (QuestEventType != EQuestEventType::CropWithered)
		return false;

	return !RequiredCrop || RequiredCrop == CropData;
}

bool UQuest::ShouldRespondToSeedPlanted(USeedDataAsset* SeedData) const
{
	if (QuestEventType != EQuestEventType::SeedPlanted)
		return false;

	return !RequiredSeed || RequiredSeed == SeedData;
}

void UQuest::StartQuest(FQuestProgress& Progress) const
{
	if (Progress.State == EQuestState::NotStarted)
		Progress.State = EQuestState::InProgress;
}

void UQuest::AddProgress(FQuestProgress& Progress, int32 Amount) const
{
	if (Progress.State != EQuestState::InProgress || Amount <= 0)
		return;

	Progress.CurrentProgress = FMath::Min(Progress.CurrentProgress + Amount, RequiredProgress);
	if (Progress.CurrentProgress >= RequiredProgress)
		Progress.State = EQuestState::Completed;
}

void UQuest::FailQuest(FQuestProgress& Progress) const
{
	if (Progress.State == EQuestState::InProgress)
		Progress.State = EQuestState::Failed;
}
