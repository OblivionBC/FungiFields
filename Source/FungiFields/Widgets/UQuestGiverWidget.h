#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UQuestGiverWidget.generated.h"

class AQuestGiverActor;
class UQuestComponent;
class UVerticalBox;
class UTextBlock;
class UButton;

UCLASS()
class FUNGIFIELDS_API UQuestGiverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quest Giver")
	void SetQuestGiver(AQuestGiverActor* QuestGiver, UQuestComponent* QuestComponent);

protected:
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Quest Giver")
	void RefreshQuests();

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void HandleQuestsUpdated();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GiverNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> QuestListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(EditDefaultsOnly, Category = "Quest Giver")
	TSubclassOf<UUserWidget> QuestEntryWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<AQuestGiverActor> BoundQuestGiver;

	UPROPERTY()
	TObjectPtr<UQuestComponent> BoundQuestComponent;
};
