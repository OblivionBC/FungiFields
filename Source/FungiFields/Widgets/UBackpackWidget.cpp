#include "UBackpackWidget.h"
#include "../Characters/FungiFieldsCharacter.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "UInventorySlotWidget.h"
#include "UInventoryDragDropOperation.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Button.h"
#include "Blueprint/DragDropOperation.h"

UBackpackWidget::UBackpackWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GridColumns = 9;
}

void UBackpackWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind close button if it exists
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UBackpackWidget::OnCloseButtonClicked);
	}

	BindToInventoryComponent();
}

FReply UBackpackWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Close on Escape or Tab
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Tab)
	{
		CloseBackpack();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UBackpackWidget::OnCloseButtonClicked()
{
	CloseBackpack();
}

void UBackpackWidget::CloseBackpack()
{
	RemoveFromParent();
	OnBackpackClosed.Broadcast();
}

void UBackpackWidget::BindToInventoryComponent()
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

	// Bind to dynamic delegate - ensure widget is fully constructed
	if (IsInViewport() || GetWorld())
	{
		InventoryComp->OnInventoryChanged.AddDynamic(this, &UBackpackWidget::OnInventoryChanged);
	}

	UpdateAllSlots();
}

void UBackpackWidget::OnInventoryChanged()
{
	UpdateAllSlots();
}

void UBackpackWidget::UpdateAllSlots()
{
	if (!CachedInventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBackpackWidget: CachedInventoryComponent is null!"));
		return;
	}
	
	if (!InventoryGrid)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBackpackWidget: InventoryGrid widget not found! Make sure your Blueprint has a UniformGridPanel named 'InventoryGrid'."));
		return;
	}

	const TArray<FInventorySlot>& InventorySlots = CachedInventoryComponent->GetInventorySlots();
	const int32 EquippedSlotIndex = CachedInventoryComponent->GetEquippedSlot();
	
	UE_LOG(LogTemp, Log, TEXT("UBackpackWidget: Updating %d slots, inventory has %d slots"), TotalSlotCount, InventorySlots.Num());

	// Ensure we have enough slot widgets
	if (SlotWidgets.Num() < TotalSlotCount)
	{
		SlotWidgets.SetNum(TotalSlotCount);
	}

	// Update all slots
	for (int32 i = 0; i < TotalSlotCount; ++i)
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

		const bool bIsEquipped = (i == EquippedSlotIndex && i < 9); // Only first 9 slots can be equipped (hotbar)
		SlotWidget->SetSlotData(SlotData, i, bIsEquipped);
		SlotWidget->SetInventorySource(0); // 0 = Player inventory
	}
}

UInventorySlotWidget* UBackpackWidget::GetOrCreateSlotWidget(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= TotalSlotCount)
	{
		return nullptr;
	}

	if (SlotWidgets[SlotIndex] && SlotWidgets[SlotIndex]->IsValidLowLevel())
	{
		return SlotWidgets[SlotIndex];
	}

	if (!InventoryGrid)
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
		UE_LOG(LogTemp, Warning, TEXT("UBackpackWidget: SlotWidgetClass not set! Creating default slot widget. Please set SlotWidgetClass in Blueprint."));
		UWorld* World = GetWorld();
		if (World)
		{
			// Use CreateWidget instead of NewObject for proper widget initialization
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, UInventorySlotWidget::StaticClass());
		}
	}

	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UBackpackWidget: Failed to create slot widget!"));
		return nullptr;
	}

	// Add to grid first - this ensures the widget is properly parented
	UUniformGridSlot* GridSlot = InventoryGrid->AddChildToUniformGrid(SlotWidget);
	if (GridSlot)
	{
		const int32 Row = SlotIndex / GridColumns;
		const int32 Column = SlotIndex % GridColumns;
		GridSlot->SetRow(Row);
		GridSlot->SetColumn(Column);
	}

	// Store widget reference
	SlotWidgets[SlotIndex] = SlotWidget;

	// Bind delegates - ensure widget and this widget are valid
	// CreateWidget should have called NativeConstruct, but let's be safe
	if (IsValid(SlotWidget) && IsValid(this) && GetWorld())
	{
		// Clear any existing bindings first to avoid duplicates
		SlotWidget->OnSlotClicked.Clear();
		SlotWidget->OnDragStarted.Clear();
		SlotWidget->OnSlotDropped.Clear();
		
		// Bind using AddDynamic - these functions are now marked as UFUNCTION
		// Use a try-catch equivalent by checking if binding is safe
		if (SlotWidget->IsValidLowLevel() && this->IsValidLowLevel())
		{
			SlotWidget->OnSlotClicked.AddDynamic(this, &UBackpackWidget::HandleSlotClicked);
			SlotWidget->OnDragStarted.AddDynamic(this, &UBackpackWidget::HandleDragStarted);
			SlotWidget->OnSlotDropped.AddDynamic(this, &UBackpackWidget::HandleSlotDropped);
		}
	}

	return SlotWidget;
}

void UBackpackWidget::HandleSlotClicked(int32 SlotIndex, int32 InventorySourceID)
{
	// Handle slot click (e.g., quick move, context menu, etc.)
	// For now, we'll handle drag & drop via the drag system
}

void UBackpackWidget::HandleDragStarted(int32 SlotIndex, int32 InventorySourceID, const FInventorySlot& SlotData)
{
	// Drag started - the actual drop handling will be in NativeOnDrop
	// This is just for tracking
}

void UBackpackWidget::HandleSlotDropped(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID)
{
	// Handle item transfer within player inventory
	if (SourceInventoryID == 0 && TargetInventoryID == 0 && CachedInventoryComponent)
	{
		CachedInventoryComponent->MoveItemToSlot(SourceSlotIndex, TargetSlotIndex);
	}
}

AFungiFieldsCharacter* UBackpackWidget::GetPlayerCharacter() const
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

