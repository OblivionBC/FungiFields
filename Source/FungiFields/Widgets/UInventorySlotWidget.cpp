#include "UInventorySlotWidget.h"
#include "UInventoryDragDropOperation.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "../Data/UItemDataAsset.h"
#include "Engine/Texture2D.h"
#include "Blueprint/DragDropOperation.h"
#include "Slate/SlateBrushAsset.h"
#include "Components/UniformGridPanel.h"
#include "Blueprint/WidgetTree.h"

UInventorySlotWidget::UInventorySlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Ensure border is found - try root widget if not bound
	if (!SlotBorder)
	{
		SlotBorder = Cast<UBorder>(GetRootWidget());
	}
	
	// Try to find ItemIcon and ItemCount if not bound
	if (!ItemIcon && WidgetTree)
	{
		// Search for Image widget using WidgetTree
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
		// Search for TextBlock widget using WidgetTree
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

void UInventorySlotWidget::UpdateSlotVisuals()
{
	// Ensure we have the border widget - it's required for visual display
	if (!SlotBorder)
	{
		// Try to find it if it wasn't bound
		SlotBorder = Cast<UBorder>(GetRootWidget());
		if (!SlotBorder)
		{
			// Log warning but don't crash
			UE_LOG(LogTemp, Warning, TEXT("UInventorySlotWidget: SlotBorder widget not found! Make sure your Blueprint has a Border widget named 'SlotBorder' as the root."));
			return;
		}
	}

	// Update equipped state visual with border
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
		NormalBrush.Margin = FMargin(2.0f); // Increased margin for visible border
		NormalBrush.TintColor = FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f)); // Darker border for visibility
		SlotBorder->SetBrush(NormalBrush);
		SlotBorder->SetPadding(FMargin(2.0f));
	}

	// Update item icon
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

	// Update item count
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
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnSlotClicked.Broadcast(SlotIndex, InventorySourceID);
		TSharedPtr<SWidget> SlateWidget = GetCachedWidget();
		if (SlateWidget.IsValid())
		{
			return FReply::Handled().DetectDrag(SlateWidget.ToSharedRef(), EKeys::LeftMouseButton);
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

	// Create custom drag operation
	UInventoryDragDropOperation* DragOperation = NewObject<UInventoryDragDropOperation>(GetWorld());
	if (DragOperation)
	{
		DragOperation->SourceSlotIndex = SlotIndex;
		DragOperation->SourceInventoryID = InventorySourceID;
		DragOperation->SlotData = CurrentSlotData;
		
		// Set default drag visual - create a simple visual representation
		if (SlotBorder)
		{
			DragOperation->DefaultDragVisual = SlotBorder;
		}
		else
		{
			DragOperation->DefaultDragVisual = this;
		}
		DragOperation->Pivot = EDragPivot::MouseDown;
		
		// Broadcast drag started event
		OnDragStarted.Broadcast(SlotIndex, InventorySourceID, CurrentSlotData);
		OutOperation = DragOperation;
	}
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& MyGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UInventoryDragDropOperation* InventoryDragOp = Cast<UInventoryDragDropOperation>(InOperation))
	{
		// Broadcast drop event to parent widget
		OnSlotDropped.Broadcast(
			InventoryDragOp->SourceSlotIndex,
			InventoryDragOp->SourceInventoryID,
			SlotIndex,
			InventorySourceID
		);
		
		bIsDragTarget = false;
		UpdateSlotVisuals();
		return true;
	}
	
	bIsDragTarget = false;
	UpdateSlotVisuals();
	return false;
}

void UInventorySlotWidget::NativeOnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	bIsDragTarget = true;
	UpdateSlotVisuals();
}

void UInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	bIsDragTarget = false;
	UpdateSlotVisuals();
}

