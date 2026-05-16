#include "USporeJournalWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "../Components/QuestComponent.h"
#include "../Data/Quest.h"
#include "../Data/USporeJournalDataAsset.h"
#include "../Data/UMilestoneDataAsset.h"
#include "../Subsystems/UMilestoneSubsystem.h"
#include "../ENUM/EQuestState.h"

void USporeJournalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &USporeJournalWidget::OnCloseButtonClicked);
	}

	// Bind to quest component on the owning player
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			BoundQuestComponent = Pawn->FindComponentByClass<UQuestComponent>();
			if (BoundQuestComponent)
			{
				BoundQuestComponent->OnQuestsUpdated.AddDynamic(this, &USporeJournalWidget::HandleQuestsUpdated);
			}
		}
	}

	// Register milestone data and bind delegate
	if (UWorld* World = GetWorld())
	{
		if (UMilestoneSubsystem* MilestoneSubsystem = World->GetSubsystem<UMilestoneSubsystem>())
		{
			if (JournalData)
			{
				MilestoneSubsystem->RegisterMilestoneData(JournalData);
			}

			MilestoneSubsystem->OnMilestoneReached.AddDynamic(this, &USporeJournalWidget::HandleMilestoneReached);
		}
	}

	RefreshQuests();
	RefreshMilestones();
}

void USporeJournalWidget::NativeDestruct()
{
	if (BoundQuestComponent)
	{
		BoundQuestComponent->OnQuestsUpdated.RemoveDynamic(this, &USporeJournalWidget::HandleQuestsUpdated);
	}

	if (UWorld* World = GetWorld())
	{
		if (UMilestoneSubsystem* MilestoneSubsystem = World->GetSubsystem<UMilestoneSubsystem>())
		{
			MilestoneSubsystem->OnMilestoneReached.RemoveDynamic(this, &USporeJournalWidget::HandleMilestoneReached);
		}
	}

	Super::NativeDestruct();
}

void USporeJournalWidget::RefreshQuests()
{
	if (!QuestListBox)
		return;

	QuestListBox->ClearChildren();

	if (!BoundQuestComponent)
		return;

	BoundQuestComponent->ForEachQuestDefinition([this](const UQuest* Quest)
	{
		if (!Quest)
			return;

		FQuestProgress Progress;
		BoundQuestComponent->GetQuestProgressForID(Quest->QuestID, Progress);

		FString StatusStr;
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

		UTextBlock* EntryText = NewObject<UTextBlock>(this);
		EntryText->SetText(FText::FromString(StatusStr));
		QuestListBox->AddChild(EntryText);
	});
}

void USporeJournalWidget::RefreshMilestones()
{
	if (!MilestoneListBox || !JournalData)
		return;

	MilestoneListBox->ClearChildren();

	UWorld* World = GetWorld();
	UMilestoneSubsystem* MilestoneSubsystem = World ? World->GetSubsystem<UMilestoneSubsystem>() : nullptr;

	for (UMilestoneDataAsset* Milestone : JournalData->Milestones)
	{
		if (!IsValid(Milestone))
			continue;

		const int32 Progress = MilestoneSubsystem ? MilestoneSubsystem->GetProgress(Milestone->MilestoneID) : 0;

		int32 CurrentTier = -1;
		int32 NextThreshold = -1;

		for (int32 i = 0; i < Milestone->TierThresholds.Num(); ++i)
		{
			if (Progress >= Milestone->TierThresholds[i])
			{
				CurrentTier = i;
			}
			else
			{
				NextThreshold = Milestone->TierThresholds[i];
				break;
			}
		}

		FString EntryStr;
		if (CurrentTier >= 0 && Milestone->TierDescriptions.IsValidIndex(CurrentTier))
		{
			EntryStr = FString::Printf(TEXT("%s — %s (%d)"),
				*Milestone->DisplayName.ToString(),
				*Milestone->TierDescriptions[CurrentTier].ToString(),
				Progress);
		}
		else
		{
			EntryStr = FString::Printf(TEXT("%s — %d"), *Milestone->DisplayName.ToString(), Progress);
		}

		if (NextThreshold > 0)
		{
			EntryStr += FString::Printf(TEXT(" / next: %d"), NextThreshold);
		}

		UTextBlock* EntryText = NewObject<UTextBlock>(this);
		EntryText->SetText(FText::FromString(EntryStr));
		MilestoneListBox->AddChild(EntryText);
	}
}

void USporeJournalWidget::OnCloseButtonClicked()
{
	OnSporeJournalClosed.Broadcast();
}

void USporeJournalWidget::HandleQuestsUpdated()
{
	RefreshQuests();
}

void USporeJournalWidget::HandleMilestoneReached(FName MilestoneID, int32 Tier)
{
	RefreshMilestones();
}
