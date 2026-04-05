#include "QuestMenu.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "QuestEntryWidget.h"
#include "FungiFields/Components/QuestComponent.h"
#include "FungiFields/Data/Quest.h"

void UQuestMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UQuestMenu::CloseMenu);

	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		BoundQuestComponent = Pawn->FindComponentByClass<UQuestComponent>();
		if (BoundQuestComponent)
			BoundQuestComponent->OnQuestsUpdated.AddDynamic(this, &UQuestMenu::HandleQuestsUpdated);
	}
}

void UQuestMenu::NativeDestruct()
{
	if (BoundQuestComponent)
	{
		BoundQuestComponent->OnQuestsUpdated.RemoveDynamic(this, &UQuestMenu::HandleQuestsUpdated);
		BoundQuestComponent = nullptr;
	}

	Super::NativeDestruct();
}

void UQuestMenu::HandleQuestsUpdated()
{
	if (GetVisibility() != ESlateVisibility::Visible)
		return;

	RefreshQuests();
}

void UQuestMenu::RefreshQuests()
{
	if (!QuestList || !BoundQuestComponent)
		return;

	QuestList->ClearChildren();

	BoundQuestComponent->ForEachQuestDefinition([this](const UQuest* Def)
	{
		FQuestProgress Progress;
		BoundQuestComponent->GetQuestProgressForID(Def->QuestID, Progress);

		if (UQuestEntryWidget* Entry = CreateWidget<UQuestEntryWidget>(GetOwningPlayer(), QuestEntryWidgetClass))
		{
			Entry->Setup(Def, Progress);
			QuestList->AddChild(Entry);
		}
	});
}

void UQuestMenu::CloseMenu()
{
	OnQuestMenuClosed.Broadcast();
}
