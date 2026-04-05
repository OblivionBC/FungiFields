#include "QuestEntryWidget.h"
#include "Components/TextBlock.h"
#include "FungiFields/ENUM/QuestState.h"

void UQuestEntryWidget::Setup(const UQuest* QuestDef, const FQuestProgress& Progress)
{
	if (!QuestDef)
		return;

	if (QuestNameText)
		QuestNameText->SetText(FText::FromName(QuestDef->QuestName));

	if (ProgressText)
	{
		ProgressText->SetText(FText::FromString(
			FString::Printf(TEXT("%d / %d"), Progress.CurrentProgress, QuestDef->RequiredProgress)
		));
	}

	if (StateText)
	{
		FString StateStr;
		switch (Progress.State)
		{
		case EQuestState::NotStarted: StateStr = TEXT("Not Started"); break;
		case EQuestState::InProgress: StateStr = TEXT("In Progress"); break;
		case EQuestState::Completed:  StateStr = TEXT("Completed");   break;
		case EQuestState::Failed:     StateStr = TEXT("Failed");      break;
		default:                      StateStr = TEXT("Unknown");     break;
		}
		StateText->SetText(FText::FromString(StateStr));
	}
}
