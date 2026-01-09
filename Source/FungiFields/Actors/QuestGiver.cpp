#include "QuestGiver.h"

#include "FungiFields/Components/QuestComponent.h"
#include "FungiFields/Data/Quest.h"


AQuestGiver::AQuestGiver()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AQuestGiver::BeginPlay()
{
	Super::BeginPlay();
	
}

void AQuestGiver::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor || !QuestData)
	{
		return;
	}

	if (UQuestComponent* QuestComponent = Interactor->FindComponentByClass<UQuestComponent>())
	{
		QuestComponent->AddQuest(QuestData);
	}
	
}

FText AQuestGiver::GetInteractionText_Implementation()
{
	return FText::FromString("Press E to Get Quest");
}

FText AQuestGiver::GetTooltipText_Implementation() const
{
	return FText::FromString("Press E to Get Quest");
}

