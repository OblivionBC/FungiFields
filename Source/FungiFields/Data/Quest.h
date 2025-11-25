#pragma once

#include "CoreMinimal.h"
#include "FungiFields/ENUM/QuestState.h"
#include "UObject/Object.h"
#include "Quest.generated.h"

UCLASS()
class FUNGIFIELDS_API UQuest : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	FName QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	int32 RequiredProgress = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Quest")
	EQuestState State = EQuestState::NotStarted;

	UFUNCTION(BlueprintCallable, Category="Quest")
	void StartQuest();

	UFUNCTION(BlueprintCallable, Category="Quest")
	void AddProgress(int32 Amount);

	UFUNCTION(BlueprintCallable, Category="Quest")
	void FailQuest();
};
