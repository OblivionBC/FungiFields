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

	UpdateSlotVisuals();
}

void UInventorySlotsWidget::OnInventoryChanged()
{
	UpdateSlotVisuals();
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

	UOverlay* SlotOverlay = NewObject<UOverlay>(this);
	
	UImage* ItemIcon = NewObject<UImage>(this);
	ItemIcon->SetBrushFromTexture(nullptr); // Will be set by UpdateSlotWidget
	ItemIcon->SetVisibility(ESlateVisibility::Collapsed); // Hidden by default
	
	UTextBlock* ItemCount = NewObject<UTextBlock>(this);
	ItemCount->SetText(FText::GetEmpty());
	ItemCount->SetVisibility(ESlateVisibility::Collapsed); // Hidden by default
	ItemCount->SetJustification(ETextJustify::Right);
	
	SlotOverlay->AddChild(ItemIcon);
	SlotOverlay->AddChild(ItemCount);
	
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
	}
	else
	{
		SlotBorder->SetBrushColor(FLinearColor::White);
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
		if (UImage* Image = Cast<UImage>(SlotOverlay->GetChildAt(i)))
		{
			ItemIcon = Image;
		}
		else if (UTextBlock* Text = Cast<UTextBlock>(SlotOverlay->GetChildAt(i)))
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
		if (ItemIcon)
		{
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
			
			if (SlotData.ItemDefinition && SlotData.ItemDefinition->ItemIcon.IsValid())
			{
				UTexture2D* IconTexture = SlotData.ItemDefinition->ItemIcon.LoadSynchronous();
				if (IconTexture)
				{
					ItemIcon->SetBrushFromTexture(IconTexture);
				}
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

AFungiFieldsCharacter* UInventorySlotsWidget::GetPlayerCharacter() const
{
	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		return Cast<AFungiFieldsCharacter>(OwningPawn);
	}
	return nullptr;
}