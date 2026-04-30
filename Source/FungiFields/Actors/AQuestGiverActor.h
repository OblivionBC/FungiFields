#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "../Interfaces/InteractableInterface.h"
#include "AQuestGiverActor.generated.h"

class UQuest;

/**
 * Placeable actor that offers quests to the player on interaction.
 * Adds offered quests to the player's UQuestComponent and opens the quest giver UI.
 */
UCLASS()
class FUNGIFIELDS_API AQuestGiverActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AQuestGiverActor();

	// IInteractableInterface
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;
	virtual FText GetTooltipText_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Giver")
	TArray<TObjectPtr<UQuest>> QuestsToOffer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Giver")
	FText VillagerName;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> QuestGiverWidgetClass;
};
