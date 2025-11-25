#include "Quest.h"

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