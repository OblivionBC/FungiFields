#include "UInventorySlotWidget.h"
#include "UItemTooltipWidget.h"
#include "UInventoryDragDropOperation.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "../Data/UItemDataAsset.h"
#include "Engine/Texture2D.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/UniformGridPanel.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetTree.h"

UInventorySlotWidget::UInventorySlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!SlotBorder)
	{
		SlotBorder = Cast<UBorder>(GetRootWidget());
	}
	
	if (!ItemIcon && WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if (!ItemIcon)
			{
				if (UImage* Image = Cast<UImage>(Widget))
				{
					ItemIcon = Image;
				}
			}
		});
	}
	
	if (!ItemCount && WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if (!ItemCount)
			{
				if (UTextBlock* Text = Cast<UTextBlock>(Widget))
				{
					ItemCount = Text;
				}
			}
		});
	}
	
	if (!SlotBorder)
	{
		CreateWidgetStructure();
	}
	
	UpdateSlotVisuals();
}

void UInventorySlotWidget::SetSlotData(const FInventorySlot& SlotData, int32 InSlotIndex, bool bInIsEquipped)
{
	CurrentSlotData = SlotData;
	SlotIndex = InSlotIndex;
	bIsEquipped = bInIsEquipped;
	UpdateSlotVisuals();
}

void UInventorySlotWidget::SetEquipped(bool bInIsEquipped)
{
	bIsEquipped = bInIsEquipped;
	UpdateSlotVisuals();
}

void UInventorySlotWidget::CreateWidgetStructure()
{
	SlotBorder = NewObject<UBorder>(this);
	SlotBorder->SetPadding(FMargin(2.0f));
	
	FSlateBrush DefaultBrush;
	DefaultBrush.DrawAs = ESlateBrushDrawType::Box;
	DefaultBrush.Margin = FMargin(2.0f);
	DefaultBrush.TintColor = FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
	SlotBorder->SetBrush(DefaultBrush);
	SlotBorder->SetBrushColor(FLinearColor::White);
	
	UOverlay* SlotOverlay = NewObject<UOverlay>(this);
	
	if (!ItemIcon)
	{
		ItemIcon = NewObject<UImage>(this);
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		ItemIcon->SetBrushTintColor(FSlateColor(FLinearColor::White));
	}
	
	if (!ItemCount)
	{
		ItemCount = NewObject<UTextBlock>(this);
		ItemCount->SetText(FText::GetEmpty());
		ItemCount->SetVisibility(ESlateVisibility::Collapsed);
		ItemCount->SetJustification(ETextJustify::Right);
	}
	
	UOverlaySlot* IconSlot = SlotOverlay->AddChildToOverlay(ItemIcon);
	if (IconSlot)
	{
		IconSlot->SetHorizontalAlignment(HAlign_Fill);
		IconSlot->SetVerticalAlignment(VAlign_Fill);
	}
	
	UOverlaySlot* CountSlot = SlotOverlay->AddChildToOverlay(ItemCount);
	if (CountSlot)
	{
		CountSlot->SetHorizontalAlignment(HAlign_Right);
		CountSlot->SetVerticalAlignment(VAlign_Bottom);
		CountSlot->SetPadding(FMargin(4.0f));
	}
	
	SlotBorder->AddChild(SlotOverlay);
	
	if (WidgetTree)
	{
		WidgetTree->RootWidget = SlotBorder;
	}
}

void UInventorySlotWidget::UpdateSlotVisuals()
{
	if (!SlotBorder)
	{
		SlotBorder = Cast<UBorder>(GetRootWidget());
		if (!SlotBorder)
		{
			CreateWidgetStructure();
			if (!SlotBorder)
			{
				UE_LOG(LogTemp, Warning, TEXT("UInventorySlotWidget: Failed to create SlotBorder widget!"));
				return;
			}
		}
	}

	if (bIsEquipped)
	{
		SlotBorder->SetBrushColor(FLinearColor::Yellow);
		FSlateBrush EquippedBrush;
		EquippedBrush.DrawAs = ESlateBrushDrawType::Box;
		EquippedBrush.Margin = FMargin(3.0f);
		EquippedBrush.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
		SlotBorder->SetBrush(EquippedBrush);
		SlotBorder->SetPadding(FMargin(2.0f));
	}
	else if (bIsDragTarget)
	{
		SlotBorder->SetBrushColor(FLinearColor::Green);
		FSlateBrush TargetBrush;
		TargetBrush.DrawAs = ESlateBrushDrawType::Box;
		TargetBrush.Margin = FMargin(2.0f);
		TargetBrush.TintColor = FSlateColor(FLinearColor(0.0f, 0.5f, 0.0f, 1.0f));
		SlotBorder->SetBrush(TargetBrush);
		SlotBorder->SetPadding(FMargin(2.0f));
	}
	else
	{
		SlotBorder->SetBrushColor(FLinearColor::White);
		FSlateBrush NormalBrush;
		NormalBrush.DrawAs = ESlateBrushDrawType::Box;
		NormalBrush.Margin = FMargin(2.0f);
		NormalBrush.TintColor = FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
		SlotBorder->SetBrush(NormalBrush);
		SlotBorder->SetPadding(FMargin(2.0f));
	}

	if (ItemIcon)
	{
		if (CurrentSlotData.IsEmpty() || !CurrentSlotData.ItemDefinition)
		{
			ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
			ItemIcon->SetBrushFromTexture(nullptr);
		}
		else
		{
			if (UTexture2D* IconTexture = CurrentSlotData.ItemDefinition->ItemIcon)
			{
				ItemIcon->SetBrushFromTexture(IconTexture, true);
				FSlateBrush Brush = ItemIcon->GetBrush();
				Brush.ImageSize = FVector2D(IconTexture->GetSizeX(), IconTexture->GetSizeY());
				Brush.DrawAs = ESlateBrushDrawType::Image;
				Brush.Tiling = ESlateBrushTileType::NoTile;
				Brush.ImageType = ESlateBrushImageType::FullColor;
				ItemIcon->SetBrush(Brush);
				ItemIcon->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	if (ItemCount)
	{
		if (CurrentSlotData.Count > 1)
		{
			ItemCount->SetVisibility(ESlateVisibility::Visible);
			ItemCount->SetText(FText::AsNumber(CurrentSlotData.Count));
		}
		else
		{
			ItemCount->SetVisibility(ESlateVisibility::Collapsed);
			ItemCount->SetText(FText::GetEmpty());
		}
	}

	if (SlotBorder)
	{
		if (CurrentSlotData.IsEmpty() || !CurrentSlotData.ItemDefinition)
		{
			SlotBorder->SetToolTip(nullptr);
		}
		else
		{
			const UItemDataAsset* ItemDef = CurrentSlotData.ItemDefinition;
			if (!CachedItemTooltip)
			{
				CachedItemTooltip = CreateWidget<UItemTooltipWidget>(this, UItemTooltipWidget::StaticClass());
			}
			if (CachedItemTooltip)
			{
				CachedItemTooltip->SetContent(ItemDef->ItemName, ItemDef->ItemDescription);
				SlotBorder->SetToolTip(CachedItemTooltip);
			}
		}
	}
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnSlotClicked.Broadcast(SlotIndex, InventorySourceID);
		// Only start drag detection when the slot holds an item. Starting detection on an empty slot
		// calls NativeOnDragDetected which returns early (OutOperation=null), leaving Slate's drag
		// state machine in a partial state that can cause subsequent drags to silently fail.
		if (!CurrentSlotData.IsEmpty())
		{
			TSharedPtr<SWidget> SlateWidget = GetCachedWidget();
			if (SlateWidget.IsValid())
			{
				return FReply::Handled().DetectDrag(SlateWidget.ToSharedRef(), EKeys::LeftMouseButton);
			}
		}
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, UDragDropOperation*& OutOperation)
{
	if (CurrentSlotData.IsEmpty())
	{
		return;
	}

	UInventoryDragDropOperation* DragOperation = NewObject<UInventoryDragDropOperation>(this);
	if (!DragOperation)
	{
		return;
	}

	DragOperation->SourceSlotIndex = SlotIndex;
	DragOperation->SourceInventoryID = InventorySourceID;
	DragOperation->SlotData = CurrentSlotData;
	DragOperation->Pivot = EDragPivot::MouseDown;

	// Create a separate widget for the drag visual using the same Blueprint class as this slot,
	// so it inherits the correct styling. Using 'this' as the visual was causing the drag system
	// to leave the source slot in an ambiguous state and interfere with drop detection.
	if (APlayerController* PC = GetOwningPlayer())
	{
		UInventorySlotWidget* DragVisual = CreateWidget<UInventorySlotWidget>(PC, GetClass());
		if (DragVisual)
		{
			DragVisual->SetSlotData(CurrentSlotData, SlotIndex, false);
			DragOperation->DefaultDragVisual = DragVisual;
		}
		else
		{
			DragOperation->DefaultDragVisual = this;
		}
	}
	else
	{
		DragOperation->DefaultDragVisual = this;
	}

	OnDragStarted.Broadcast(SlotIndex, InventorySourceID, CurrentSlotData);
	OutOperation = DragOperation;
}

bool UInventorySlotWidget::NativeOnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation))
	{
		// Reject hovering over the slot that is the drag source.
		if (DragOp->SourceSlotIndex == SlotIndex && DragOp->SourceInventoryID == InventorySourceID)
			return false;
		return true;
	}
	return false;
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& MyGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UInventoryDragDropOperation* InventoryDragOp = Cast<UInventoryDragDropOperation>(InOperation))
	{
		bIsDragTarget = false;
		UpdateSlotVisuals();

		// Silently ignore a drop onto the originating slot.
		if (InventoryDragOp->SourceSlotIndex == SlotIndex && InventoryDragOp->SourceInventoryID == InventorySourceID)
			return false;

		OnSlotDropped.Broadcast(
			InventoryDragOp->SourceSlotIndex,
			InventoryDragOp->SourceInventoryID,
			SlotIndex,
			InventorySourceID
		);
		return true;
	}

	bIsDragTarget = false;
	UpdateSlotVisuals();
	return false;
}

void UInventorySlotWidget::NativeOnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// Don't highlight the source slot when drag begins over it.
	if (UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation))
	{
		if (DragOp->SourceSlotIndex == SlotIndex && DragOp->SourceInventoryID == InventorySourceID)
			return;
	}
	bIsDragTarget = true;
	UpdateSlotVisuals();
}

void UInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	bIsDragTarget = false;
	UpdateSlotVisuals();
}



