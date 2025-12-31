#include "InventorySlotsWidget.h"
#include "../Characters/FungiFieldsCharacter.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "../Data/UItemDataAsset.h"
#include "Engine/Texture2D.h"

constexpr int32 UInventorySlotsWidget::HotbarSize;

void UInventorySlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToInventoryComponent();
}

void UInventorySlotsWidget::BindToInventoryComponent()
{
	AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	UInventoryComponent* InventoryComp = PlayerCharacter->InventoryComponent;
	if (!InventoryComp)
	{
		return;
	}

	CachedInventoryComponent = InventoryComp;

	InventoryComp->OnInventoryChanged.AddDynamic(this, &UInventorySlotsWidget::OnInventoryChanged);
	InventoryComp->OnItemEquipped.AddDynamic(this, &UInventorySlotsWidget::OnItemEquipped);

	UpdateSlotVisuals();
	UpdateEquippedItemName();
}

void UInventorySlotsWidget::OnInventoryChanged()
{
	UpdateSlotVisuals();
	UpdateEquippedItemName();
}

void UInventorySlotsWidget::OnItemEquipped(UItemDataAsset* Item, int32 SlotIndex)
{
	UpdateEquippedItemName();
}

void UInventorySlotsWidget::UpdateSlotVisuals()
{
	if (!CachedInventoryComponent || !SlotsContainer)
	{
		return;
	}

	const TArray<FInventorySlot>& InventorySlots = CachedInventoryComponent->GetInventorySlots();
	const int32 EquippedSlotIndex = CachedInventoryComponent->GetEquippedSlot();

	if (SlotWidgets.Num() < HotbarSize)
	{
		SlotWidgets.SetNum(HotbarSize);
	}

	for (int32 i = 0; i < HotbarSize; ++i)
	{
		UWidget* SlotWidget = GetOrCreateSlotWidget(i);
		
		FInventorySlot SlotData;
		if (i < InventorySlots.Num())
		{
			SlotData = InventorySlots[i];
		}

		const bool bIsEquipped = (i == EquippedSlotIndex);
		UpdateSlotWidget(SlotWidget, SlotData, i, bIsEquipped);
	}
}

UWidget* UInventorySlotsWidget::GetOrCreateSlotWidget(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= HotbarSize)
	{
		return nullptr;
	}

	if (SlotWidgets[SlotIndex] && SlotWidgets[SlotIndex]->IsValidLowLevel())
	{
		return SlotWidgets[SlotIndex];
	}

	if (!SlotsContainer)
	{
		return nullptr;
	}

	UBorder* SlotBorder = NewObject<UBorder>(this);
	SlotBorder->SetPadding(FMargin(2.0f));
	
	FSlateBrush DefaultBrush;
	DefaultBrush.DrawAs = ESlateBrushDrawType::Box;
	DefaultBrush.Margin = FMargin(1.0f);
	DefaultBrush.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f)); // Dark gray background
	SlotBorder->SetBrush(DefaultBrush);
	SlotBorder->SetBrushColor(FLinearColor::White);

	UOverlay* SlotOverlay = NewObject<UOverlay>(this);
	
	UImage* ItemIcon = NewObject<UImage>(this);
	ItemIcon->SetBrushFromTexture(nullptr);
	ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	ItemIcon->SetBrushTintColor(FSlateColor(FLinearColor::White));
	
	UTextBlock* ItemCount = NewObject<UTextBlock>(this);
	ItemCount->SetText(FText::GetEmpty());
	ItemCount->SetVisibility(ESlateVisibility::Collapsed);
	ItemCount->SetJustification(ETextJustify::Right);
	
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
	
	UHorizontalBoxSlot* SlotBoxSlot = SlotsContainer->AddChildToHorizontalBox(SlotBorder);
	if (SlotBoxSlot)
	{
		SlotBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		SlotBoxSlot->SetPadding(FMargin(2.0f));
	}

	SlotWidgets[SlotIndex] = SlotBorder;
	return SlotBorder;
}

void UInventorySlotsWidget::UpdateSlotWidget(UWidget* SlotWidget, const FInventorySlot& SlotData, int32 SlotIndex, bool bIsEquipped)
{
	if (!SlotWidget)
	{
		return;
	}

	UBorder* SlotBorder = Cast<UBorder>(SlotWidget);
	if (!SlotBorder)
	{
		return;
	}

	if (bIsEquipped)
	{
		SlotBorder->SetBrushColor(FLinearColor::Yellow);
		FSlateBrush EquippedBrush;
		EquippedBrush.DrawAs = ESlateBrushDrawType::Box;
		EquippedBrush.Margin = FMargin(3.0f);
		EquippedBrush.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
		SlotBorder->SetBrush(EquippedBrush);
	}
	else
	{
		SlotBorder->SetBrushColor(FLinearColor::White);
		FSlateBrush NormalBrush;
		NormalBrush.DrawAs = ESlateBrushDrawType::Box;
		NormalBrush.Margin = FMargin(1.0f);
		NormalBrush.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
		SlotBorder->SetBrush(NormalBrush);
	}

	UOverlay* SlotOverlay = nullptr;
	if (SlotBorder->GetChildrenCount() > 0)
	{
		SlotOverlay = Cast<UOverlay>(SlotBorder->GetChildAt(0));
	}

	if (!SlotOverlay)
	{
		return;
	}

	UImage* ItemIcon = nullptr;
	UTextBlock* ItemCount = nullptr;

	for (int32 i = 0; i < SlotOverlay->GetChildrenCount(); ++i)
	{
		UWidget* Child = SlotOverlay->GetChildAt(i);
		if (UImage* Image = Cast<UImage>(Child))
		{
			ItemIcon = Image;
		}
		else if (UTextBlock* Text = Cast<UTextBlock>(Child))
		{
			ItemCount = Text;
		}
	}

	if (SlotData.IsEmpty())
	{
		if (ItemIcon)
		{
			ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
			ItemIcon->SetBrushFromTexture(nullptr);
		}
		if (ItemCount)
		{
			ItemCount->SetVisibility(ESlateVisibility::Collapsed);
			ItemCount->SetText(FText::GetEmpty());
		}
	}
	else
	{
		if (ItemIcon && SlotData.ItemDefinition)
		{
			if (UTexture2D* IconTexture = SlotData.ItemDefinition->ItemIcon)
			{
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
			}
			else
			{
				ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		if (ItemCount)
		{
			if (SlotData.Count > 1)
			{
				ItemCount->SetVisibility(ESlateVisibility::Visible);
				ItemCount->SetText(FText::AsNumber(SlotData.Count));
			}
			else
			{
				ItemCount->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void UInventorySlotsWidget::UpdateEquippedItemName()
{
	if (!CachedInventoryComponent)
	{
		if (EquippedItemNameText)
		{
			EquippedItemNameText->SetText(FText::GetEmpty());
			EquippedItemNameText->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	const int32 EquippedSlotIndex = CachedInventoryComponent->GetEquippedSlot();
	const TArray<FInventorySlot>& InventorySlots = CachedInventoryComponent->GetInventorySlots();

	if (!EquippedItemNameText)
	{
		return;
	}

	// Check if there's an equipped slot with an item
	if (EquippedSlotIndex != INDEX_NONE && 
		InventorySlots.IsValidIndex(EquippedSlotIndex) && 
		!InventorySlots[EquippedSlotIndex].IsEmpty() &&
		InventorySlots[EquippedSlotIndex].ItemDefinition)
	{
		const FInventorySlot& EquippedSlot = InventorySlots[EquippedSlotIndex];
		if (EquippedSlot.ItemDefinition)
		{
			EquippedItemNameText->SetText(EquippedSlot.ItemDefinition->ItemName);
			EquippedItemNameText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			EquippedItemNameText->SetText(FText::GetEmpty());
			EquippedItemNameText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		// No item equipped
		EquippedItemNameText->SetText(FText::GetEmpty());
		EquippedItemNameText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

AFungiFieldsCharacter* UInventorySlotsWidget::GetPlayerCharacter() const
{
	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		AFungiFieldsCharacter* Character = Cast<AFungiFieldsCharacter>(OwningPawn);
		return Character;
	}
	
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				return Cast<AFungiFieldsCharacter>(Pawn);
			}
		}
	}
	return nullptr;
}