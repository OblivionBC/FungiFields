#include "UItemTooltipWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Fonts/SlateFontInfo.h"
#include "Brushes/SlateColorBrush.h"

void UItemTooltipWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetStructure();
}

void UItemTooltipWidget::BuildWidgetStructure()
{
	if (!WidgetTree || NameText)
	{
		return;
	}

	UBorder* BackgroundBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	if (!BackgroundBorder)
	{
		return;
	}

	const FLinearColor BackgroundColor(0.05f, 0.05f, 0.08f, 0.95f);
	BackgroundBorder->SetBrush(FSlateColorBrush(BackgroundColor));
	BackgroundBorder->SetBrushColor(BackgroundColor);
	BackgroundBorder->SetPadding(FMargin(12.0f, 8.0f));

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (!ContentBox)
	{
		return;
	}

	NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	DescriptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (!NameText || !DescriptionText)
	{
		return;
	}

	FSlateFontInfo NameFont = NameText->GetFont();
	NameFont.Size = NameFontSize;
	NameText->SetFont(NameFont);
	NameText->SetText(FText::GetEmpty());
	NameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

	FSlateFontInfo DescFont = DescriptionText->GetFont();
	DescFont.Size = DescriptionFontSize;
	DescriptionText->SetFont(DescFont);
	DescriptionText->SetText(FText::GetEmpty());
	DescriptionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.0f)));

	UVerticalBoxSlot* NameSlot = ContentBox->AddChildToVerticalBox(NameText);
	if (NameSlot)
	{
		NameSlot->SetPadding(FMargin(0.0f, 0.0f));
	}
	UVerticalBoxSlot* DescSlot = ContentBox->AddChildToVerticalBox(DescriptionText);
	if (DescSlot)
	{
		DescSlot->SetPadding(FMargin(0.0f, 4.0f));
	}

	BackgroundBorder->AddChild(ContentBox);
	WidgetTree->RootWidget = BackgroundBorder;
}

void UItemTooltipWidget::SetContent(const FText& Name, const FText& Description)
{
	BuildWidgetStructure();
	if (NameText)
	{
		NameText->SetText(Name);
	}
	if (DescriptionText)
	{
		DescriptionText->SetText(Description);
		DescriptionText->SetVisibility(Description.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}
