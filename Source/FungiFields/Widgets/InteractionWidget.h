#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionWidget.generated.h"

class UTextBlock;
class AActor;

/**
 * A lightweight interaction prompt widget.
 * Create a UMG Widget Blueprint derived from this class to lay out visuals.
 */
UCLASS(BlueprintType, Blueprintable)
class UInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetPromptText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ShowPrompt();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HidePrompt();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateFromActor(AActor* Interactable);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PromptText;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* ShowAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* HideAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText DefaultPromptText = FText::FromString(TEXT("Interact"));
};
