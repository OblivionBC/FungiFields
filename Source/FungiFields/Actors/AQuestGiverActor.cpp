#include "AQuestGiverActor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "../Components/QuestComponent.h"
#include "../Widgets/UQuestGiverWidget.h"
#include "../Data/Quest.h"

AQuestGiverActor::AQuestGiverActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AQuestGiverActor::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor || !QuestGiverWidgetClass)
		return;

	APawn* InteractorPawn = Cast<APawn>(Interactor);
	APlayerController* PC = InteractorPawn ? Cast<APlayerController>(InteractorPawn->GetController()) : nullptr;
	if (!PC)
		return;

	UQuestComponent* QuestComp = Interactor->FindComponentByClass<UQuestComponent>();
	if (QuestComp)
	{
		for (UQuest* Quest : QuestsToOffer)
		{
			if (IsValid(Quest))
			{
				QuestComp->AddQuest(Quest);
			}
		}
	}

	UQuestGiverWidget* Widget = CreateWidget<UQuestGiverWidget>(PC, QuestGiverWidgetClass);
	if (!Widget)
		return;

	Widget->SetQuestGiver(this, QuestComp);
	Widget->AddToViewport();
}

FText AQuestGiverActor::GetInteractionText_Implementation()
{
	return FText::Format(INVTEXT("Talk to {0}"), VillagerName);
}

FText AQuestGiverActor::GetTooltipText_Implementation() const
{
	return FText::Format(INVTEXT("Talk to {0}"), VillagerName);
}
