#include "QuestComponent.h"
#include "../Data/Quest.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();
}

UQuest* UQuestComponent::AddQuest(UQuest* Quest)
{
	if (!Quest)
		return nullptr;

	ActiveQuests.Add(Quest->QuestID, Quest);

	Quest->StartQuest();

	return Quest;
}

TArray<UQuest*> UQuestComponent::GetAllQuests() const
{
	TArray<UQuest*> Values;
	ActiveQuests.GenerateValueArray(Values);
	return Values;
}

bool UQuestComponent::RemoveQuest(FName QuestID)
{
	return ActiveQuests.Remove(QuestID) > 0;
}

