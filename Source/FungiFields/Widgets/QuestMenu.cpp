#include "QuestMenu.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "QuestEntryWidget.h"
#include "FungiFields/Components/QuestComponent.h"
#include "FungiFields/Data/Quest.h"
#include "GameFramework/PlayerController.h"

void UQuestMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UQuestMenu::CloseMenu);
	}
}

void UQuestMenu::RefreshQuests()
{
	if (!QuestList)
		return;

	QuestList->ClearChildren();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
		return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
		return;

	UQuestComponent* QC = Pawn->FindComponentByClass<UQuestComponent>();
	if (!QC)
		return;

	TArray<UQuest*> Quests = QC->GetAllQuests();

	for (UQuest* Quest : Quests)
	{
		UQuestEntryWidget* Entry = CreateWidget<UQuestEntryWidget>(GetWorld(), QuestEntryWidgetClass);

		if (Entry)
		{
			Entry->Setup(Quest);
			QuestList->AddChild(Entry);
		}
	}
}

void UQuestMenu::CloseMenu()
{
	OnQuestMenuClosed.Broadcast();
}