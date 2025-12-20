#include "UChestWidget.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "UInventorySlotWidget.h"
#include "UInventoryDragDropOperation.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"

UChestWidget::UChestWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GridColumns = 9;
}

void UChestWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind close button if it exists
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UChestWidget::OnCloseButtonClicked);
	}
}

void UChestWidget::OnCloseButtonClicked()
{
	CloseWidget();
}

void UChestWidget::SetupInventories(UInventoryComponent* PlayerInventoryComp, UInventoryComponent* ChestInventoryComp)
{
	PlayerInventory = PlayerInventoryComp;
	ChestInventory = ChestInventoryComp;

	if (PlayerInventory)
	{
		// Bind to dynamic delegate - ensure widget is fully constructed
		if (IsInViewport() || GetWorld())
		{
			PlayerInventory->OnInventoryChanged.AddDynamic(this, &UChestWidget::OnPlayerInventoryChanged);
		}
		UpdatePlayerSlots();
	}

	if (ChestInventory)
	{
		// Bind to dynamic delegate - ensure widget is fully constructed
		if (IsInViewport() || GetWorld())
		{
			ChestInventory->OnInventoryChanged.AddDynamic(this, &UChestWidget::OnChestInventoryChanged);
		}
		UpdateChestSlots();
	}
}

void UChestWidget::CloseWidget()
{
	RemoveFromParent();

	// Restore game input mode
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
		}
	}

	// Broadcast close event via delegate
	OnChestWidgetClosed.Broadcast();
}

FReply UChestWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Close on Escape or Tab
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Tab)
	{
		CloseWidget();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UChestWidget::OnPlayerInventoryChanged()
{
	UpdatePlayerSlots();
}

void UChestWidget::OnChestInventoryChanged()
{
	UpdateChestSlots();
}

void UChestWidget::UpdatePlayerSlots()
{
	if (!PlayerInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: PlayerInventory is null!"));
		return;
	}
	
	if (!PlayerInventoryGrid)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: PlayerInventoryGrid widget not found! Make sure your Blueprint has a UniformGridPanel named 'PlayerInventoryGrid'."));
		return;
	}

	const TArray<FInventorySlot>& InventorySlots = PlayerInventory->GetInventorySlots();
	const int32 EquippedSlotIndex = PlayerInventory->GetEquippedSlot();
	
	UE_LOG(LogTemp, Log, TEXT("UChestWidget: Updating %d player slots, inventory has %d slots"), PlayerSlotCount, InventorySlots.Num());

	// Ensure we have enough slot widgets
	if (PlayerSlotWidgets.Num() < PlayerSlotCount)
	{
		PlayerSlotWidgets.SetNum(PlayerSlotCount);
	}

	// Update all player slots
	for (int32 i = 0; i < PlayerSlotCount; ++i)
	{
		UInventorySlotWidget* SlotWidget = GetOrCreatePlayerSlotWidget(i);
		if (!SlotWidget)
		{
			continue;
		}

		FInventorySlot SlotData;
		if (i < InventorySlots.Num())
		{
			SlotData = InventorySlots[i];
		}

		const bool bIsEquipped = (i == EquippedSlotIndex && i < 9); // Only first 9 slots can be equipped (hotbar)
		SlotWidget->SetSlotData(SlotData, i, bIsEquipped);
		SlotWidget->SetInventorySource(0); // 0 = Player inventory
	}
}

void UChestWidget::UpdateChestSlots()
{
	if (!ChestInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: ChestInventory is null!"));
		return;
	}
	
	if (!ChestInventoryGrid)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: ChestInventoryGrid widget not found! Make sure your Blueprint has a UniformGridPanel named 'ChestInventoryGrid'."));
		return;
	}

	const TArray<FInventorySlot>& InventorySlots = ChestInventory->GetInventorySlots();
	
	UE_LOG(LogTemp, Log, TEXT("UChestWidget: Updating %d chest slots, inventory has %d slots"), ChestSlotCount, InventorySlots.Num());

	// Ensure we have enough slot widgets
	if (ChestSlotWidgets.Num() < ChestSlotCount)
	{
		ChestSlotWidgets.SetNum(ChestSlotCount);
	}

	// Update all chest slots
	for (int32 i = 0; i < ChestSlotCount; ++i)
	{
		UInventorySlotWidget* SlotWidget = GetOrCreateChestSlotWidget(i);
		if (!SlotWidget)
		{
			continue;
		}

		FInventorySlot SlotData;
		if (i < InventorySlots.Num())
		{
			SlotData = InventorySlots[i];
		}

		SlotWidget->SetSlotData(SlotData, i, false); // Chest slots are never equipped
		SlotWidget->SetInventorySource(1); // 1 = Chest inventory
	}
}

UInventorySlotWidget* UChestWidget::GetOrCreatePlayerSlotWidget(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= PlayerSlotCount)
	{
		return nullptr;
	}

	if (PlayerSlotWidgets[SlotIndex] && PlayerSlotWidgets[SlotIndex]->IsValidLowLevel())
	{
		return PlayerSlotWidgets[SlotIndex];
	}

	if (!PlayerInventoryGrid)
	{
		return nullptr;
	}

	// Create slot widget
	UInventorySlotWidget* SlotWidget = nullptr;
	if (SlotWidgetClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, SlotWidgetClass);
		}
	}
	
	// Fallback: create default if no class specified or creation failed
	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: SlotWidgetClass not set! Creating default slot widget. Please set SlotWidgetClass in Blueprint."));
		UWorld* World = GetWorld();
		if (World)
		{
			// Use CreateWidget instead of NewObject for proper widget initialization
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, UInventorySlotWidget::StaticClass());
		}
	}

	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UChestWidget: Failed to create slot widget!"));
		return nullptr;
	}

	// Add to grid first
	UUniformGridSlot* GridSlot = PlayerInventoryGrid->AddChildToUniformGrid(SlotWidget);
	if (GridSlot)
	{
		const int32 Row = SlotIndex / GridColumns;
		const int32 Column = SlotIndex % GridColumns;
		GridSlot->SetRow(Row);
		GridSlot->SetColumn(Column);
	}

	// Store widget reference
	PlayerSlotWidgets[SlotIndex] = SlotWidget;

	// Bind delegates - ensure widget and this widget are valid
	if (IsValid(SlotWidget) && IsValid(this) && GetWorld())
	{
		// Clear any existing bindings first to avoid duplicates
		SlotWidget->OnSlotClicked.Clear();
		SlotWidget->OnDragStarted.Clear();
		SlotWidget->OnSlotDropped.Clear();
		
		// Bind using AddDynamic - these functions are now marked as UFUNCTION
		if (SlotWidget->IsValidLowLevel() && this->IsValidLowLevel())
		{
			SlotWidget->OnSlotClicked.AddDynamic(this, &UChestWidget::HandlePlayerSlotClicked);
			SlotWidget->OnDragStarted.AddDynamic(this, &UChestWidget::HandlePlayerDragStarted);
			SlotWidget->OnSlotDropped.AddDynamic(this, &UChestWidget::HandleSlotDropped);
		}
	}

	return SlotWidget;
}

UInventorySlotWidget* UChestWidget::GetOrCreateChestSlotWidget(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= ChestSlotCount)
	{
		return nullptr;
	}

	if (ChestSlotWidgets[SlotIndex] && ChestSlotWidgets[SlotIndex]->IsValidLowLevel())
	{
		return ChestSlotWidgets[SlotIndex];
	}

	if (!ChestInventoryGrid)
	{
		return nullptr;
	}

	// Create slot widget
	UInventorySlotWidget* SlotWidget = nullptr;
	if (SlotWidgetClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, SlotWidgetClass);
		}
	}
	
	// Fallback: create default if no class specified or creation failed
	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: SlotWidgetClass not set! Creating default slot widget. Please set SlotWidgetClass in Blueprint."));
		UWorld* World = GetWorld();
		if (World)
		{
			// Use CreateWidget instead of NewObject for proper widget initialization
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, UInventorySlotWidget::StaticClass());
		}
	}

	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UChestWidget: Failed to create slot widget!"));
		return nullptr;
	}

	// Add to grid first
	UUniformGridSlot* GridSlot = ChestInventoryGrid->AddChildToUniformGrid(SlotWidget);
	if (GridSlot)
	{
		const int32 Row = SlotIndex / GridColumns;
		const int32 Column = SlotIndex % GridColumns;
		GridSlot->SetRow(Row);
		GridSlot->SetColumn(Column);
	}

	// Store widget reference
	ChestSlotWidgets[SlotIndex] = SlotWidget;

	// Bind delegates - ensure widget and this widget are valid
	if (IsValid(SlotWidget) && IsValid(this) && GetWorld())
	{
		// Clear any existing bindings first to avoid duplicates
		SlotWidget->OnSlotClicked.Clear();
		SlotWidget->OnDragStarted.Clear();
		SlotWidget->OnSlotDropped.Clear();
		
		// Bind using AddDynamic - these functions are now marked as UFUNCTION
		if (SlotWidget->IsValidLowLevel() && this->IsValidLowLevel())
		{
			SlotWidget->OnSlotClicked.AddDynamic(this, &UChestWidget::HandleChestSlotClicked);
			SlotWidget->OnDragStarted.AddDynamic(this, &UChestWidget::HandleChestDragStarted);
			SlotWidget->OnSlotDropped.AddDynamic(this, &UChestWidget::HandleSlotDropped);
		}
	}

	return SlotWidget;
}

void UChestWidget::HandlePlayerSlotClicked(int32 SlotIndex, int32 InventorySourceID)
{
	// Handle slot click
}

void UChestWidget::HandleChestSlotClicked(int32 SlotIndex, int32 InventorySourceID)
{
	// Handle slot click
}

void UChestWidget::HandlePlayerDragStarted(int32 SlotIndex, int32 InventorySourceID, const FInventorySlot& SlotData)
{
	// Drag started from player inventory
}

void UChestWidget::HandleChestDragStarted(int32 SlotIndex, int32 InventorySourceID, const FInventorySlot& SlotData)
{
	// Drag started from chest inventory
}

void UChestWidget::HandleSlotDropped(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID)
{
	HandleItemTransfer(SourceSlotIndex, SourceInventoryID, TargetSlotIndex, TargetInventoryID);
}

bool UChestWidget::HandleItemTransfer(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID)
{
	// Player to Player (rearrange within player inventory)
	if (SourceInventoryID == 0 && TargetInventoryID == 0)
	{
		if (PlayerInventory)
		{
			return PlayerInventory->MoveItemToSlot(SourceSlotIndex, TargetSlotIndex);
		}
	}
	// Chest to Chest (rearrange within chest inventory)
	else if (SourceInventoryID == 1 && TargetInventoryID == 1)
	{
		if (ChestInventory)
		{
			// Direct swap for chest (replication handled by component)
			return ChestInventory->SwapSlots(SourceSlotIndex, TargetSlotIndex);
		}
	}
	// Player to Chest
	else if (SourceInventoryID == 0 && TargetInventoryID == 1)
	{
		if (PlayerInventory && ChestInventory)
		{
			const TArray<FInventorySlot>& PlayerSlots = PlayerInventory->GetInventorySlots();
			if (PlayerSlots.IsValidIndex(SourceSlotIndex) && !PlayerSlots[SourceSlotIndex].IsEmpty())
			{
				const FInventorySlot& SourceSlot = PlayerSlots[SourceSlotIndex];
				if (SourceSlot.ItemDefinition)
				{
					// Try to add to chest
					if (ChestInventory->TryAddItem(const_cast<UItemDataAsset*>(SourceSlot.ItemDefinition.Get()), SourceSlot.Count))
					{
						// Remove from player inventory
						PlayerInventory->ConsumeFromSlot(SourceSlotIndex, SourceSlot.Count);
						return true;
					}
				}
			}
		}
	}
	// Chest to Player
	else if (SourceInventoryID == 1 && TargetInventoryID == 0)
	{
		if (PlayerInventory && ChestInventory)
		{
			const TArray<FInventorySlot>& ChestSlots = ChestInventory->GetInventorySlots();
			if (ChestSlots.IsValidIndex(SourceSlotIndex) && !ChestSlots[SourceSlotIndex].IsEmpty())
			{
				const FInventorySlot& SourceSlot = ChestSlots[SourceSlotIndex];
				if (SourceSlot.ItemDefinition)
				{
					// Try to add to player inventory
					if (PlayerInventory->TryAddItem(const_cast<UItemDataAsset*>(SourceSlot.ItemDefinition.Get()), SourceSlot.Count))
					{
						// Remove from chest
						ChestInventory->RemoveFromSlot(SourceSlotIndex, SourceSlot.Count);
						return true;
					}
				}
			}
		}
	}

	return false;
}

