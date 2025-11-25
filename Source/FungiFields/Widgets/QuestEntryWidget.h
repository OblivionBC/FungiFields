#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestEntryWidget.generated.h"

class UTextBlock;
class UQuest;

UCLASS()
class FUNGIFIELDS_API UQuestEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup(UQuest* InQuest);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuestNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ProgressText;

private:
	UPROPERTY()
	UQuest* Quest;
};
