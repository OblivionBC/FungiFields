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
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
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
		NormalBrush.Margin = FMargin(2.0f); // Increased margin for visible border
		NormalBrush.TintColor = FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f)); // Darker border for visibility
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

void UInventorySlotWidget::EnsureDragVisualCreated()
{
	if (CachedDragVisual && CachedDragSizeBox)
	{
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	// Create SizeBox to control the drag visual size
	CachedDragSizeBox = NewObject<USizeBox>(GetWorld());
	if (!CachedDragSizeBox)
	{
		return;
	}

	// Create Border as the visual container
	CachedDragVisual = NewObject<UBorder>(GetWorld());
	if (!CachedDragVisual)
	{
		return;
	}

	FSlateBrush DefaultBrush;
	DefaultBrush.DrawAs = ESlateBrushDrawType::Box;
	DefaultBrush.Margin = FMargin(2.0f);
	DefaultBrush.TintColor = FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
	CachedDragVisual->SetBrush(DefaultBrush);
	CachedDragVisual->SetPadding(FMargin(2.0f));

	// Create Image for the item icon
	CachedDragIcon = NewObject<UImage>(GetWorld());
	if (CachedDragIcon)
	{
		CachedDragVisual->AddChild(CachedDragIcon);
	}

	// Add Border to SizeBox
	CachedDragSizeBox->AddChild(CachedDragVisual);
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, UDragDropOperation*& OutOperation)
{
	if (CurrentSlotData.IsEmpty())
	{
		return;
	}

	UInventoryDragDropOperation* DragOperation = NewObject<UInventoryDragDropOperation>(GetWorld());
	if (DragOperation)
	{
		DragOperation->SourceSlotIndex = SlotIndex;
		DragOperation->SourceInventoryID = InventorySourceID;
		DragOperation->SlotData = CurrentSlotData;
		
		if (SlotBorder)
		{
			EnsureDragVisualCreated();
			
			if (CachedDragSizeBox && CachedDragVisual)
			{
				// Get the slot size from the geometry
				FVector2D SlotSize = MyGeometry.GetLocalSize();
				
				// Set fixed size on the SizeBox to control drag visual size
				CachedDragSizeBox->SetWidthOverride(SlotSize.X);
				CachedDragSizeBox->SetHeightOverride(SlotSize.Y);
				
				// Update the visual appearance
				CachedDragVisual->SetBrushColor(SlotBorder->GetBrushColor());
				
				if (CachedDragIcon && CurrentSlotData.ItemDefinition && CurrentSlotData.ItemDefinition->ItemIcon)
				{
					CachedDragIcon->SetBrushFromTexture(CurrentSlotData.ItemDefinition->ItemIcon, true);
					CachedDragIcon->SetVisibility(ESlateVisibility::Visible);
				}
				else if (CachedDragIcon)
				{
					CachedDragIcon->SetVisibility(ESlateVisibility::Collapsed);
				}
				
				DragOperation->DefaultDragVisual = CachedDragSizeBox;
			}
			else
			{
				DragOperation->DefaultDragVisual = SlotBorder;
			}
		}
		else
		{
			DragOperation->DefaultDragVisual = this;
		}
		DragOperation->Pivot = EDragPivot::MouseDown;
		
		OnDragStarted.Broadcast(SlotIndex, InventorySourceID, CurrentSlotData);
		OutOperation = DragOperation;
	}
}

bool UInventorySlotWidget::NativeOnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (Cast<UInventoryDragDropOperation>(InOperation))
	{
		return true;
	}
	return false;
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& MyGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UInventoryDragDropOperation* InventoryDragOp = Cast<UInventoryDragDropOperation>(InOperation))
	{
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



