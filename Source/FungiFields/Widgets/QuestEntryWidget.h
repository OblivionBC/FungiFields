#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FungiFields/Data/Quest.h"
#include "QuestEntryWidget.generated.h"

class UTextBlock;

UCLASS()
class FUNGIFIELDS_API UQuestEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup(const UQuest* QuestDef, const FQuestProgress& Progress);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuestNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ProgressText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateText;
};
