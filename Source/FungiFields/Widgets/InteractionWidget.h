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
	// Set the prompt text, e.g. "Press E to Interact"
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetPromptText(const FText& InText);

	// Show/Hide with optional animations
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ShowPrompt();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HidePrompt();

	// Optional helper if you want to push actor info in (e.g., name or interface text)
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateFromActor(AActor* Interactable);
	
	// Expose to C++ logic
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PromptText;
	
protected:
	virtual void NativeConstruct() override;

	// Optional animations (name them "ShowAnim" and "HideAnim" in your Widget Blueprint)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* ShowAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* HideAnim;

	// Default text if none provided
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText DefaultPromptText = FText::FromString(TEXT("Interact"));
};
