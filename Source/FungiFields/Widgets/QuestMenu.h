#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestMenu.generated.h"

class UVerticalBox;
class UQuestComponent;

UCLASS()
class FUNGIFIELDS_API UQuestMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void RefreshQuests();

protected:
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* QuestList;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UQuestEntryWidget> QuestEntryWidgetClass;
};
