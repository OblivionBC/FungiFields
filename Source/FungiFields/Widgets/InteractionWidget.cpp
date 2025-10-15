#include "InteractionWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"

void UInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize with default text
	if (PromptText && !DefaultPromptText.IsEmpty())
	{
		PromptText->SetText(DefaultPromptText);
	}

	// Start hidden until shown
	SetVisibility(ESlateVisibility::Hidden);
}

void UInteractionWidget::SetPromptText(const FText& InText)
{
	if (PromptText)
	{
		PromptText->SetText(InText.IsEmpty() ? DefaultPromptText : InText);
	}
}

void UInteractionWidget::ShowPrompt()
{
	// Make visible and play an optional animation
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (ShowAnim)
	{
		PlayAnimation(ShowAnim, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f, false);
	}
}

void UInteractionWidget::HidePrompt()
{
	if (HideAnim)
	{
		// When a hide animation finishes, you may want to set Hidden; for simplicity, set immediately.
		PlayAnimation(HideAnim, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f, false);
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UInteractionWidget::UpdateFromActor(AActor* Interactable)
{
	// Optional: pull text from an interface or actor properties.
	// Example (if you add GetInteractionText() to your interface):
	// if (Interactable && Interactable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	// {
	//     const FText Text = IInteractableInterface::Execute_GetInteractionText(Interactable);
	//     SetPromptText(Text);
	// }
	// else
	// {
	//     SetPromptText(DefaultPromptText);
	// }
}
