#include "UQuestEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "FungiFields/ENUM/EQuestState.h"
#include "FungiFields/Components/QuestComponent.h"

void UQuestEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CollectButton)
	{
		CollectButton->OnClicked.AddDynamic(this, &UQuestEntryWidget::OnCollectButtonClicked);
		CollectButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UQuestEntryWidget::NativeDestruct()
{
	if (CollectButton)
	{
		CollectButton->OnClicked.RemoveDynamic(this, &UQuestEntryWidget::OnCollectButtonClicked);
	}
	Super::NativeDestruct();
}

void UQuestEntryWidget::SetQuestComponent(UQuestComponent* InQuestComponent)
{
	BoundQuestComponent = InQuestComponent;
}

void UQuestEntryWidget::Setup(const UQuest* QuestDef, const FQuestProgress& Progress)
{
	if (!QuestDef)
		return;

	CachedQuestID = QuestDef->QuestID;

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
		case EQuestState::NotStarted:    StateStr = TEXT("Not Started");    break;
		case EQuestState::InProgress:    StateStr = TEXT("In Progress");    break;
		case EQuestState::ReadyToCollect:StateStr = TEXT("Ready to Collect"); break;
		case EQuestState::Completed:     StateStr = TEXT("Completed");      break;
		case EQuestState::Failed:        StateStr = TEXT("Failed");         break;
		default:                         StateStr = TEXT("Unknown");        break;
		}
		StateText->SetText(FText::FromString(StateStr));
	}

	if (CollectButton)
	{
		const bool bShowCollect = (Progress.State == EQuestState::ReadyToCollect);
		CollectButton->SetVisibility(bShowCollect ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UQuestEntryWidget::OnCollectButtonClicked()
{
	if (BoundQuestComponent && CachedQuestID != NAME_None)
	{
		BoundQuestComponent->CollectQuestReward(CachedQuestID);
	}
}
