#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FungiFields/Data/Quest.h"
#include "UQuestEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UQuestComponent;

UCLASS()
class FUNGIFIELDS_API UQuestEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup(const UQuest* QuestDef, const FQuestProgress& Progress);

	/** Optional: set the owning quest component so the collect button can dispatch rewards. */
	UFUNCTION(BlueprintCallable)
	void SetQuestComponent(UQuestComponent* InQuestComponent);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuestNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ProgressText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateText;

	UPROPERTY(meta = (BindWidget))
	UButton* CollectButton;

private:
	UFUNCTION()
	void OnCollectButtonClicked();

	UPROPERTY()
	TObjectPtr<UQuestComponent> BoundQuestComponent;

	FName CachedQuestID;
};
