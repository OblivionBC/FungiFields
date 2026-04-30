#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USporeJournalWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSporeJournalClosed);

class USporeJournalDataAsset;
class UQuestComponent;
class UVerticalBox;
class UButton;

/**
 * Main journal UI with two tabs: Quests and Milestones.
 * Assign JournalData in the Blueprint default to enable milestone tab.
 * Open/dismiss via player character Blueprint key binding.
 */
UCLASS()
class FUNGIFIELDS_API USporeJournalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spore Journal")
	TObjectPtr<USporeJournalDataAsset> JournalData;

	UPROPERTY(BlueprintAssignable)
	FOnSporeJournalClosed OnSporeJournalClosed;
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Spore Journal")
	void RefreshQuests();

	UFUNCTION(BlueprintCallable, Category = "Spore Journal")
	void RefreshMilestones();

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void HandleQuestsUpdated();

	UFUNCTION()
	void HandleMilestoneReached(FName MilestoneID, int32 Tier);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> QuestListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> MilestoneListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

private:
	UPROPERTY()
	TObjectPtr<UQuestComponent> BoundQuestComponent;
};
