// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
class UQuest;
#include "QuestComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	UQuest* AddQuest(UQuest* QuestClass);

	UFUNCTION(BlueprintCallable)
	TArray<UQuest*> GetAllQuests() const;

	UFUNCTION(BlueprintCallable)
	bool RemoveQuest(FName QuestID);

private:
	UPROPERTY()
	TMap<FName, UQuest*> ActiveQuests;
};
