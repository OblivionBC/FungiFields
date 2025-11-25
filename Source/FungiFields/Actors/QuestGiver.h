// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this actor's properties
	AQuestGiver();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Interaction interface
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	UQuest* QuestData;
};
