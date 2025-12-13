#include "Quest.h"
#include "../Data/UItemDataAsset.h"
#include "../Data/UCropDataAsset.h"
#include "../Data/USeedDataAsset.h"

bool UQuest::ShouldRespondToItemAdded(UItemDataAsset* Item, int32 Quantity) const
{
	if (QuestEventType != EQuestEventType::ItemAdded)
	{
		return false;
	}

	// If no specific item required, respond to any item
	if (!RequiredItem.IsValid())
	{
		return true;
	}

	// Check if the added item matches the required item
	UItemDataAsset* RequiredItemPtr = RequiredItem.LoadSynchronous();
	return RequiredItemPtr && Item == RequiredItemPtr;
}

bool UQuest::ShouldRespondToCropHarvested(UCropDataAsset* CropData, int32 Quantity) const
{
	if (QuestEventType != EQuestEventType::CropHarvested)
	{
		return false;
	}

	// If no specific crop required, respond to any crop
	if (!RequiredCrop.IsValid())
	{
		return true;
	}

	// Check if the harvested crop matches the required crop
	UCropDataAsset* RequiredCropPtr = RequiredCrop.LoadSynchronous();
	return RequiredCropPtr && CropData == RequiredCropPtr;
}

bool UQuest::ShouldRespondToSeedPlanted(USeedDataAsset* SeedData) const
{
	if (QuestEventType != EQuestEventType::SeedPlanted)
	{
		return false;
	}

	// If no specific seed required, respond to any seed
	if (!RequiredSeed.IsValid())
	{
		return true;
	}

	// Check if the planted seed matches the required seed
	USeedDataAsset* RequiredSeedPtr = RequiredSeed.LoadSynchronous();
	return RequiredSeedPtr && SeedData == RequiredSeedPtr;
}

void UQuest::StartQuest()
{
	if (State == EQuestState::NotStarted)
	{
		State = EQuestState::InProgress;
	}
}

void UQuest::AddProgress(int32 Amount)
{
	if (State != EQuestState::InProgress || Amount <= 0)
		return;

	CurrentProgress += Amount;

	if (CurrentProgress >= RequiredProgress)
	{
		State = EQuestState::Completed;
		CurrentProgress = RequiredProgress;
	}
}

void UQuest::FailQuest()
{
	if (State == EQuestState::InProgress)
	{
		State = EQuestState::Failed;
	}
}