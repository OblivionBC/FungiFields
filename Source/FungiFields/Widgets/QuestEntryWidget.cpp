#include "QuestEntryWidget.h"
#include "Components/TextBlock.h"
#include "FungiFields/Data/Quest.h"

void UQuestEntryWidget::Setup(UQuest* InQuest)
{
	Quest = InQuest;

	if (!Quest)
		return;

	if (QuestNameText)
		QuestNameText->SetText(FText::FromName(Quest->QuestName));

	if (ProgressText)
	{
		FString ProgressStr = FString::Printf(TEXT("%d / %d"),
			Quest->CurrentProgress,
			Quest->RequiredProgress
		);
		ProgressText->SetText(FText::FromString(ProgressStr));
	}
}
