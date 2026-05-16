#include "UPlayerInventoryWidget.h"
#include "Components/Button.h"

void UPlayerInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UPlayerInventoryWidget::OnCloseButtonClicked);
}

FReply UPlayerInventoryWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Tab)
	{
		CloseInventory();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UPlayerInventoryWidget::OnCloseButtonClicked()
{
	CloseInventory();
}

void UPlayerInventoryWidget::CloseInventory()
{
	SetVisibility(ESlateVisibility::Hidden);
	OnInventoryClosed.Broadcast();
}
