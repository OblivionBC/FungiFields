#include "InventorySlotsWidget.h"
#include "UInventorySlotWidget.h"
#include "../Characters/FungiFieldsCharacter.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "../Data/UItemDataAsset.h"
#include "InputActionValue.h"

constexpr int32 UInventorySlotsWidget::HotbarSize;

void UInventorySlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Log, TEXT("[Hotbar] NativeConstruct called."));

	if (!SlotsContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Hotbar] SlotsContainer is NULL. In your hotbar Blueprint (e.g. WBP_InventorySlots): add a Horizontal Box, set its *variable name* to exactly 'SlotsContainer'. Leave it empty."));
	}
	else
	{
		const int32 ChildCount = SlotsContainer->GetChildrenCount();
		UE_LOG(LogTemp, Log, TEXT("[Hotbar] SlotsContainer found with %d children (will use existing slot widgets)."), ChildCount);
	}

	BindToInventoryComponent();
}

void UInventorySlotsWidget::BindToInventoryComponent()
{
	AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Hotbar] GetPlayerCharacter() returned NULL. HUD may be created before pawn is possessed, or this widget has no Owning Player."));
		return;
	}

	UInventoryComponent* InventoryComp = PlayerCharacter->InventoryComponent;
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Hotbar] Player has no InventoryComponent. Add an Inventory Component to the character Blueprint."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Hotbar] Bound to inventory; calling UpdateSlotVisuals."));

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
	if (!CachedInventoryComponent)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Hotbar] UpdateSlotVisuals skipped: no CachedInventoryComponent."));
		return;
	}
	if (!SlotsContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Hotbar] UpdateSlotVisuals skipped: SlotsContainer is NULL. Slots will not be created."));
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
		UInventorySlotWidget* SlotWidget = GetOrCreateSlotWidget(i);
		if (!SlotWidget)
		{
			continue;
		}

		FInventorySlot SlotData;
		if (i < InventorySlots.Num())
		{
			SlotData = InventorySlots[i];
		}

		const bool bIsEquipped = (i == EquippedSlotIndex);
		UpdateSlotWidget(SlotWidget, SlotData, i, bIsEquipped);
	}

	UE_LOG(LogTemp, Log, TEXT("[Hotbar] UpdateSlotVisuals done: %d slots."), HotbarSize);
}

UInventorySlotWidget* UInventorySlotsWidget::GetOrCreateSlotWidget(int32 SlotIndex)
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
		UE_LOG(LogTemp, Warning, TEXT("[Hotbar] GetOrCreateSlotWidget(%d): SlotsContainer is NULL."), SlotIndex);
		return nullptr;
	}

	// Use the 9 slot widgets already placed in the Blueprint under SlotsContainer
	if (SlotIndex >= SlotsContainer->GetChildrenCount())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Hotbar] GetOrCreateSlotWidget(%d): SlotsContainer has only %d children. Add 9 Inventory Slot widgets in the Blueprint."), SlotIndex, SlotsContainer->GetChildrenCount());
		return nullptr;
	}

	UWidget* Child = SlotsContainer->GetChildAt(SlotIndex);
	UInventorySlotWidget* SlotWidget = Cast<UInventorySlotWidget>(Child);
	// If the child is a wrapper (e.g. Size Box), the actual slot may be its first child
	if (!SlotWidget && Child)
	{
		UPanelWidget* ChildPanel = Cast<UPanelWidget>(Child);
		if (ChildPanel && ChildPanel->GetChildrenCount() > 0)
		{
			SlotWidget = Cast<UInventorySlotWidget>(ChildPanel->GetChildAt(0));
		}
	}
	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Hotbar] GetOrCreateSlotWidget(%d): Child at index %d is not an Inventory Slot Widget (or a container with one). Use Inventory Slot Widget Blueprint as children of SlotsContainer."), SlotIndex, SlotIndex);
		return nullptr;
	}

	SlotWidget->SetInventorySource(0);
	SlotWidget->OnSlotClicked.AddDynamic(this, &UInventorySlotsWidget::HandleSlotClicked);

	SlotWidgets[SlotIndex] = SlotWidget;
	return SlotWidget;
}

void UInventorySlotsWidget::HandleSlotClicked(int32 SlotIndex, int32 InventorySourceID)
{
	if (CachedInventoryComponent && InventorySourceID == 0)
	{
		CachedInventoryComponent->EquipSlot(FInputActionValue(), SlotIndex);
	}
}

void UInventorySlotsWidget::UpdateSlotWidget(UInventorySlotWidget* SlotWidget, const FInventorySlot& SlotData, int32 SlotIndex, bool bIsEquipped)
{
	if (!SlotWidget)
	{
		return;
	}

	SlotWidget->SetSlotData(SlotData, SlotIndex, bIsEquipped);
	SlotWidget->SetInventorySource(0);
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