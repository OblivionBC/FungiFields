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
		if (IsInViewport() || GetWorld())
		{
			PlayerInventory->OnInventoryChanged.AddDynamic(this, &UChestWidget::OnPlayerInventoryChanged);
		}
		UpdatePlayerSlots();
	}

	if (ChestInventory)
	{
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

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
		}
	}

	OnChestWidgetClosed.Broadcast();
}

FReply UChestWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
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

	if (PlayerSlotWidgets.Num() < PlayerSlotCount)
	{
		PlayerSlotWidgets.SetNum(PlayerSlotCount);
	}

	for (int32 i = 0; i < PlayerSlotCount; ++i)
	{
		UInventorySlotWidget* SlotWidget = GetOrCreatePlayerSlotWidget(i);
		if (!SlotWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("UChestWidget: Failed to create player slot widget for slot %d"), i);
			continue;
		}

		FInventorySlot SlotData;
		if (i < InventorySlots.Num())
		{
			SlotData = InventorySlots[i];
		}

		const bool bIsEquipped = (i == EquippedSlotIndex && i < 9);
		SlotWidget->SetSlotData(SlotData, i, bIsEquipped);
		SlotWidget->SetInventorySource(0);
		
		SlotWidget->SetVisibility(ESlateVisibility::Visible);
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

	if (ChestSlotWidgets.Num() < ChestSlotCount)
	{
		ChestSlotWidgets.SetNum(ChestSlotCount);
	}

	for (int32 i = 0; i < ChestSlotCount; ++i)
	{
		UInventorySlotWidget* SlotWidget = GetOrCreateChestSlotWidget(i);
		if (!SlotWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("UChestWidget: Failed to create chest slot widget for slot %d"), i);
			continue;
		}

		FInventorySlot SlotData;
		if (i < InventorySlots.Num())
		{
			SlotData = InventorySlots[i];
		}

		SlotWidget->SetSlotData(SlotData, i, false);
		SlotWidget->SetInventorySource(1);
		
		if (SlotWidget->GetSlotIndex() != i)
		{
			UE_LOG(LogTemp, Error, TEXT("UChestWidget: Slot index mismatch! Expected %d but got %d"), i, SlotWidget->GetSlotIndex());
		}
		
		SlotWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

UInventorySlotWidget* UChestWidget::GetOrCreatePlayerSlotWidget(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= PlayerSlotCount)
	{
		return nullptr;
	}

	if (PlayerSlotWidgets.Num() <= SlotIndex)
	{
		PlayerSlotWidgets.SetNum(SlotIndex + 1);
	}

	if (PlayerSlotWidgets[SlotIndex] && PlayerSlotWidgets[SlotIndex]->IsValidLowLevel())
	{
		return PlayerSlotWidgets[SlotIndex];
	}

	if (!PlayerInventoryGrid)
	{
		return nullptr;
	}

	UInventorySlotWidget* SlotWidget = nullptr;
	if (SlotWidgetClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, SlotWidgetClass);
		}
	}
	
	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: SlotWidgetClass not set! Creating default slot widget. Please set SlotWidgetClass in Blueprint."));
		UWorld* World = GetWorld();
		if (World)
		{
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, UInventorySlotWidget::StaticClass());
		}
	}

	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UChestWidget: Failed to create slot widget!"));
		return nullptr;
	}

	SlotWidget->SetVisibility(ESlateVisibility::Visible);

	UUniformGridSlot* GridSlot = PlayerInventoryGrid->AddChildToUniformGrid(SlotWidget);
	if (GridSlot)
	{
		const int32 Row = SlotIndex / GridColumns;
		const int32 Column = SlotIndex % GridColumns;
		GridSlot->SetRow(Row);
		GridSlot->SetColumn(Column);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: Failed to add player slot widget %d to grid"), SlotIndex);
	}

	PlayerSlotWidgets[SlotIndex] = SlotWidget;

	if (IsValid(SlotWidget) && IsValid(this) && GetWorld())
	{
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

	if (ChestSlotWidgets.Num() <= SlotIndex)
	{
		ChestSlotWidgets.SetNum(SlotIndex + 1);
	}

	if (ChestSlotWidgets[SlotIndex] && ChestSlotWidgets[SlotIndex]->IsValidLowLevel())
	{
		return ChestSlotWidgets[SlotIndex];
	}

	if (!ChestInventoryGrid)
	{
		return nullptr;
	}

	UInventorySlotWidget* SlotWidget = nullptr;
	if (SlotWidgetClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, SlotWidgetClass);
		}
	}
	
	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: SlotWidgetClass not set! Creating default slot widget. Please set SlotWidgetClass in Blueprint."));
		UWorld* World = GetWorld();
		if (World)
		{
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, UInventorySlotWidget::StaticClass());
		}
	}

	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UChestWidget: Failed to create slot widget!"));
		return nullptr;
	}

	SlotWidget->SetVisibility(ESlateVisibility::Visible);

	UUniformGridSlot* GridSlot = ChestInventoryGrid->AddChildToUniformGrid(SlotWidget);
	if (GridSlot)
	{
		const int32 Row = SlotIndex / GridColumns;
		const int32 Column = SlotIndex % GridColumns;
		GridSlot->SetRow(Row);
		GridSlot->SetColumn(Column);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UChestWidget: Failed to add chest slot widget %d to grid"), SlotIndex);
	}

	ChestSlotWidgets[SlotIndex] = SlotWidget;

	if (IsValid(SlotWidget) && IsValid(this) && GetWorld())
	{
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
}

void UChestWidget::HandleChestSlotClicked(int32 SlotIndex, int32 InventorySourceID)
{
}

void UChestWidget::HandlePlayerDragStarted(int32 SlotIndex, int32 InventorySourceID, const FInventorySlot& SlotData)
{
}

void UChestWidget::HandleChestDragStarted(int32 SlotIndex, int32 InventorySourceID, const FInventorySlot& SlotData)
{
}

void UChestWidget::HandleSlotDropped(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID)
{
	HandleItemTransfer(SourceSlotIndex, SourceInventoryID, TargetSlotIndex, TargetInventoryID);
}

bool UChestWidget::HandleItemTransfer(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID)
{
	if (SourceInventoryID == 0 && TargetInventoryID == 0)
	{
		if (PlayerInventory)
		{
			return PlayerInventory->MoveItemToSlot(SourceSlotIndex, TargetSlotIndex);
		}
	}
	else if (SourceInventoryID == 1 && TargetInventoryID == 1)
	{
		if (ChestInventory)
		{
			return ChestInventory->SwapSlots(SourceSlotIndex, TargetSlotIndex);
		}
	}
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
					if (ChestInventory->TryAddItem(const_cast<UItemDataAsset*>(SourceSlot.ItemDefinition.Get()), SourceSlot.Count))
					{
						PlayerInventory->ConsumeFromSlot(SourceSlotIndex, SourceSlot.Count);
						return true;
					}
				}
			}
		}
	}
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
					if (PlayerInventory->TryAddItem(const_cast<UItemDataAsset*>(SourceSlot.ItemDefinition.Get()), SourceSlot.Count))
					{
						ChestInventory->RemoveFromSlot(SourceSlotIndex, SourceSlot.Count);
						return true;
					}
				}
			}
		}
	}

	return false;
}



