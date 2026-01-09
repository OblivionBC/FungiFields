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

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UBackpackWidget::OnCloseButtonClicked);
	}

	BindToInventoryComponent();
}

FReply UBackpackWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
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
	SetVisibility(ESlateVisibility::Hidden);
	OnBackpackClosed.Broadcast();
}

void UBackpackWidget::RefreshInventory()
{
	if (!CachedInventoryComponent)
	{
		BindToInventoryComponent();
	}
	
	UpdateAllSlots();
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

	if (SlotWidgets.Num() < TotalSlotCount)
	{
		SlotWidgets.SetNum(TotalSlotCount);
	}

	for (int32 i = 0; i < TotalSlotCount; ++i)
	{
		UInventorySlotWidget* SlotWidget = GetOrCreateSlotWidget(i);
		if (!SlotWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("UBackpackWidget: Failed to create widget for slot %d"), i);
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
		UE_LOG(LogTemp, Warning, TEXT("UBackpackWidget: SlotWidgetClass not set! Creating default slot widget. Please set SlotWidgetClass in Blueprint."));
		UWorld* World = GetWorld();
		if (World)
		{
			SlotWidget = CreateWidget<UInventorySlotWidget>(World, UInventorySlotWidget::StaticClass());
		}
	}

	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UBackpackWidget: Failed to create slot widget!"));
		return nullptr;
	}

	SlotWidget->SetVisibility(ESlateVisibility::Visible);

	UUniformGridSlot* GridSlot = InventoryGrid->AddChildToUniformGrid(SlotWidget);
	if (GridSlot)
	{
		const int32 Row = SlotIndex / GridColumns;
		const int32 Column = SlotIndex % GridColumns;
		GridSlot->SetRow(Row);
		GridSlot->SetColumn(Column);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UBackpackWidget: Failed to add slot widget %d to grid"), SlotIndex);
	}

	SlotWidgets[SlotIndex] = SlotWidget;

	if (IsValid(SlotWidget) && IsValid(this) && GetWorld())
	{
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
}

void UBackpackWidget::HandleDragStarted(int32 SlotIndex, int32 InventorySourceID, const FInventorySlot& SlotData)
{
}

void UBackpackWidget::HandleSlotDropped(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID)
{
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



