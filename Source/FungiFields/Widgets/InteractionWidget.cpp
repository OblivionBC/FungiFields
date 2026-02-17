#include "InteractionWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"

void UInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PromptText && !DefaultPromptText.IsEmpty())
	{
		PromptText->SetText(DefaultPromptText);
	}

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
		PlayAnimation(HideAnim, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f, false);
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UInteractionWidget::UpdateFromActor(AActor* Interactable)
{
}
