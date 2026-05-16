#include "UQuestGiverWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "../Actors/AQuestGiverActor.h"
#include "../Components/QuestComponent.h"
#include "../Data/Quest.h"
#include "../ENUM/EQuestState.h"

void UQuestGiverWidget::SetQuestGiver(AQuestGiverActor* QuestGiver, UQuestComponent* QuestComponent)
{
	BoundQuestGiver = QuestGiver;
	BoundQuestComponent = QuestComponent;

	if (BoundQuestComponent)
	{
		BoundQuestComponent->OnQuestsUpdated.AddDynamic(this, &UQuestGiverWidget::HandleQuestsUpdated);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UQuestGiverWidget::OnCloseButtonClicked);
	}

	if (GiverNameText && IsValid(BoundQuestGiver))
	{
		GiverNameText->SetText(BoundQuestGiver->VillagerName);
	}

	RefreshQuests();
}

void UQuestGiverWidget::NativeDestruct()
{
	if (BoundQuestComponent)
	{
		BoundQuestComponent->OnQuestsUpdated.RemoveDynamic(this, &UQuestGiverWidget::HandleQuestsUpdated);
	}

	Super::NativeDestruct();
}

void UQuestGiverWidget::RefreshQuests()
{
	if (!QuestListBox || !IsValid(BoundQuestGiver))
		return;

	QuestListBox->ClearChildren();

	for (UQuest* Quest : BoundQuestGiver->QuestsToOffer)
	{
		if (!IsValid(Quest))
			continue;

		if (BoundQuestComponent && Quest->PrerequisiteQuests.Num() > 0)
		{
			bool bAllPrereqsMet = true;
			for (const TObjectPtr<UQuest>& Prereq : Quest->PrerequisiteQuests)
			{
				if (!IsValid(Prereq.Get()))
				{
					continue;
				}

				FQuestProgress PrereqProgress;
				if (!BoundQuestComponent->GetQuestProgressForID(Prereq->QuestID, PrereqProgress)
					|| PrereqProgress.State != EQuestState::Completed)
				{
					bAllPrereqsMet = false;
					break;
				}
			}

			if (!bAllPrereqsMet)
			{
				continue;
			}
		}

		FString StatusStr;

		if (BoundQuestComponent)
		{
			FQuestProgress Progress;
			if (BoundQuestComponent->GetQuestProgressForID(Quest->QuestID, Progress))
			{
				switch (Progress.State)
				{
				case EQuestState::Completed:
					StatusStr = FString::Printf(TEXT("[Done] %s"), *Quest->QuestName.ToString());
					break;
				case EQuestState::ReadyToCollect:
					StatusStr = FString::Printf(TEXT("[Collect!] %s"), *Quest->QuestName.ToString());
					break;
				case EQuestState::Failed:
					StatusStr = FString::Printf(TEXT("[Failed] %s"), *Quest->QuestName.ToString());
					break;
				case EQuestState::InProgress:
					StatusStr = FString::Printf(TEXT("[%d/%d] %s"), Progress.CurrentProgress, Quest->RequiredProgress, *Quest->QuestName.ToString());
					break;
				default:
					StatusStr = Quest->QuestName.ToString();
					break;
				}
			}
			else
			{
				StatusStr = FString::Printf(TEXT("[Accept] %s"), *Quest->QuestName.ToString());
			}
		}
		else
		{
			StatusStr = Quest->QuestName.ToString();
		}

		UTextBlock* EntryText = NewObject<UTextBlock>(this);
		EntryText->SetText(FText::FromString(StatusStr));
		QuestListBox->AddChild(EntryText);
	}
}

void UQuestGiverWidget::OnCloseButtonClicked()
{
	RemoveFromParent();
}

void UQuestGiverWidget::HandleQuestsUpdated()
{
	RefreshQuests();
}
