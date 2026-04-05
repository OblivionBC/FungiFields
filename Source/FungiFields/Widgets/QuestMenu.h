#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestMenu.generated.h"

class UVerticalBox;
class UQuestComponent;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestMenuClosed);

UCLASS()
class FUNGIFIELDS_API UQuestMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void RefreshQuests();

	UFUNCTION(BlueprintCallable)
	void CloseMenu();

	UPROPERTY(BlueprintAssignable)
	FOnQuestMenuClosed OnQuestMenuClosed;

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleQuestsUpdated();

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* QuestList;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UQuestEntryWidget> QuestEntryWidgetClass;

	UPROPERTY()
	UQuestComponent* BoundQuestComponent = nullptr;
};
