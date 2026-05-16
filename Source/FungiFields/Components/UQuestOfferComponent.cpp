#include "UQuestOfferComponent.h"
#include "QuestComponent.h"
#include "../Data/Quest.h"
#include "../ENUM/EQuestState.h"

UQuestOfferComponent::UQuestOfferComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<UQuest*> UQuestOfferComponent::GetAvailableQuests(UQuestComponent* PlayerQuestComp) const
{
	TArray<UQuest*> Available;

	for (const TObjectPtr<UQuest>& QuestPtr : QuestsToOffer)
	{
		UQuest* Quest = QuestPtr.Get();
		if (!IsValid(Quest))
			continue;

		if (PlayerQuestComp)
		{
			// Skip if already in the player's quest log (any state)
			FQuestProgress Existing;
			if (PlayerQuestComp->GetQuestProgressForID(Quest->QuestID, Existing))
				continue;

			// Skip if prerequisite quests are not yet completed
			bool bPrereqsMet = true;
			for (const TObjectPtr<UQuest>& Prereq : Quest->PrerequisiteQuests)
			{
				if (!IsValid(Prereq.Get()))
					continue;

				FQuestProgress PrereqProgress;
				if (!PlayerQuestComp->GetQuestProgressForID(Prereq->QuestID, PrereqProgress)
					|| PrereqProgress.State != EQuestState::Completed)
				{
					bPrereqsMet = false;
					break;
				}
			}
			if (!bPrereqsMet)
				continue;
		}

		Available.Add(Quest);
	}

	return Available;
}

bool UQuestOfferComponent::HasAvailableQuests(UQuestComponent* PlayerQuestComp) const
{
	return GetAvailableQuests(PlayerQuestComp).Num() > 0;
}

int32 UQuestOfferComponent::GiveAvailableQuests(UQuestComponent* PlayerQuestComp)
{
	if (!PlayerQuestComp)
		return 0;

	int32 Added = 0;
	for (UQuest* Quest : GetAvailableQuests(PlayerQuestComp))
	{
		if (PlayerQuestComp->AddQuest(Quest))
			++Added;
	}
	return Added;
}
