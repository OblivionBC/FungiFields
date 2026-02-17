#pragma once

#include "CoreMinimal.h"
#include "FungiFields/Interfaces/InteractableInterface.h"
#include "GameFramework/Actor.h"
class UQuest;
#include "QuestGiver.generated.h"

UCLASS()
class FUNGIFIELDS_API AQuestGiver : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AQuestGiver();

protected:
	virtual void BeginPlay() override;

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;

	virtual FText GetTooltipText_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	UQuest* QuestData;
};
