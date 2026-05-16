#include "UVillagerManagementWidget.h"
#include "UBackpackWidget.h"
#include "UInventorySlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "../Components/InventoryComponent.h"
#include "../Components/UCropBedSelectionComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "../Data/UToolDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "GameFramework/Pawn.h"

void UVillagerManagementWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HarvesterButton)
		HarvesterButton->OnClicked.AddDynamic(this, &UVillagerManagementWidget::OnHarvesterButtonClicked);

	if (PlanterButton)
		PlanterButton->OnClicked.AddDynamic(this, &UVillagerManagementWidget::OnPlanterButtonClicked);

	if (WatererButton)
		WatererButton->OnClicked.AddDynamic(this, &UVillagerManagementWidget::OnWatererButtonClicked);

	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UVillagerManagementWidget::OnCloseButtonClicked);

	if (AssignBedsButton)
		AssignBedsButton->OnClicked.AddDynamic(this, &UVillagerManagementWidget::StartBedAssignment);
}

void UVillagerManagementWidget::NativeDestruct()
{
	if (BoundVillager)
	{
		BoundVillager->OnRoleChanged.RemoveDynamic(this, &UVillagerManagementWidget::HandleRoleChanged);

		if (VillagerInventoryComp)
			VillagerInventoryComp->OnInventoryChanged.RemoveDynamic(this, &UVillagerManagementWidget::HandleInventoryChanged);
	}

	if (PlayerInventoryComp)
		PlayerInventoryComp->OnInventoryChanged.RemoveDynamic(this, &UVillagerManagementWidget::OnPlayerInventoryChanged);

	Super::NativeDestruct();
}

void UVillagerManagementWidget::SetVillager(AFarmerVillagerCharacter* Villager)
{
	BoundVillager = Villager;

	if (!BoundVillager)
		return;

	BoundVillager->OnRoleChanged.AddDynamic(this, &UVillagerManagementWidget::HandleRoleChanged);

	if (UInventoryComponent* InvComp = BoundVillager->GetInventoryComponent())
	{
		VillagerInventoryComp = InvComp;
		InvComp->OnInventoryChanged.AddDynamic(this, &UVillagerManagementWidget::HandleInventoryChanged);

		if (VillagerInventoryWidget)
			VillagerInventoryWidget->SetInventoryComponent(InvComp);
	}

	SetupInventoryGrids();
	RefreshWidget();
}

void UVillagerManagementWidget::RefreshWidget()
{
	if (!BoundVillager)
		return;

	if (VillagerNameText)
	{
		VillagerNameText->SetText(BoundVillager->GetVillagerDisplayName());
	}

	RefreshRoleHighlight();
	RefreshToolWarning();
	RefreshInventorySummary();
}

void UVillagerManagementWidget::RefreshRoleHighlight()
{
	if (!BoundVillager)
		return;

	const EFarmerRole CurrentRole = BoundVillager->GetAssignedRole();

	auto SetButtonEnabled = [](UButton* Button, bool bEnabled)
	{
		if (Button)
			Button->SetIsEnabled(bEnabled);
	};

	SetButtonEnabled(HarvesterButton, CurrentRole != EFarmerRole::Harvester);
	SetButtonEnabled(PlanterButton, CurrentRole != EFarmerRole::Planter);
	SetButtonEnabled(WatererButton, CurrentRole != EFarmerRole::Waterer);
}

void UVillagerManagementWidget::RefreshToolWarning()
{
	if (!BoundVillager || !NoToolWarningText)
		return;

	const bool bHasTool = (BoundVillager->FindEquippedToolForRole() != nullptr);
	NoToolWarningText->SetVisibility(bHasTool ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UVillagerManagementWidget::RefreshInventorySummary()
{
	if (!BoundVillager || !InventorySummaryBox)
		return;

	InventorySummaryBox->ClearChildren();

	UInventoryComponent* InvComp = BoundVillager->GetInventoryComponent();
	if (!InvComp)
		return;

	for (const FInventorySlot& CurrSlot : InvComp->GetInventorySlots())
	{
		if (CurrSlot.IsEmpty())
			continue;

		FString ItemDesc;

		if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(CurrSlot.ItemDefinition.Get()))
		{
			ItemDesc = FString::Printf(TEXT("[Tool] %s x%d"), *ToolData->GetName(), CurrSlot.Count);
		}
		else if (const USeedDataAsset* SeedData = Cast<const USeedDataAsset>(CurrSlot.ItemDefinition.Get()))
		{
			ItemDesc = FString::Printf(TEXT("[Seed] %s x%d"), *SeedData->GetName(), CurrSlot.Count);
		}
		else
		{
			ItemDesc = FString::Printf(TEXT("%s x%d"), *CurrSlot.ItemDefinition->GetName(), CurrSlot.Count);
		}

		UTextBlock* EntryText = NewObject<UTextBlock>(this);
		EntryText->SetText(FText::FromString(ItemDesc));
		InventorySummaryBox->AddChild(EntryText);
	}
}

void UVillagerManagementWidget::OnHarvesterButtonClicked()
{
	if (BoundVillager)
		BoundVillager->SetAssignedRole(EFarmerRole::Harvester);
}

void UVillagerManagementWidget::OnPlanterButtonClicked()
{
	if (BoundVillager)
		BoundVillager->SetAssignedRole(EFarmerRole::Planter);
}

void UVillagerManagementWidget::OnWatererButtonClicked()
{
	if (BoundVillager)
		BoundVillager->SetAssignedRole(EFarmerRole::Waterer);
}

void UVillagerManagementWidget::StartBedAssignment()
{
	if (!BoundVillager) return;

	// Find CropBedSelectionComponent on the owning player pawn — no hard dependency on AFungiFieldsCharacter.
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (!PlayerPawn) return;

	UCropBedSelectionComponent* SelectionComp = PlayerPawn->FindComponentByClass<UCropBedSelectionComponent>();
	if (!SelectionComp) return;

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	SetVisibility(ESlateVisibility::Hidden);
	SelectionComp->StartSelection(BoundVillager);
}

void UVillagerManagementWidget::OnCloseButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
	RemoveFromParent();
}

void UVillagerManagementWidget::HandleRoleChanged(EFarmerRole NewRole)
{
	RefreshRoleHighlight();
	RefreshToolWarning();
}

void UVillagerManagementWidget::HandleInventoryChanged()
{
	RefreshToolWarning();
	RefreshInventorySummary();
	UpdateVillagerSlots();
}

void UVillagerManagementWidget::SetupInventoryGrids()
{
	if (APawn* PlayerPawn = GetOwningPlayerPawn())
	{
		PlayerInventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>();
		if (PlayerInventoryComp)
		{
			PlayerInventoryComp->OnInventoryChanged.AddDynamic(this, &UVillagerManagementWidget::OnPlayerInventoryChanged);
			UpdatePlayerSlots();
		}
	}

	UpdateVillagerSlots();
}

void UVillagerManagementWidget::OnPlayerInventoryChanged()
{
	UpdatePlayerSlots();
}

void UVillagerManagementWidget::UpdatePlayerSlots()
{
	if (!PlayerInventoryComp || !PlayerInventoryGrid) return;

	const TArray<FInventorySlot>& Slots = PlayerInventoryComp->GetInventorySlots();
	const int32 EquippedSlot = PlayerInventoryComp->GetEquippedSlot();

	PlayerSlotWidgets.SetNum(PlayerSlotCount);

	for (int32 i = 0; i < PlayerSlotCount; ++i)
	{
		UInventorySlotWidget* SlotWidget = GetOrCreatePlayerSlotWidget(i);
		if (!SlotWidget) continue;

		FInventorySlot SlotData;
		if (i < Slots.Num()) SlotData = Slots[i];

		SlotWidget->SetSlotData(SlotData, i, i == EquippedSlot && i < 9);
		SlotWidget->SetInventorySource(0);
		SlotWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UVillagerManagementWidget::UpdateVillagerSlots()
{
	if (!VillagerInventoryComp || !VillagerInventoryGrid) return;

	const TArray<FInventorySlot>& Slots = VillagerInventoryComp->GetInventorySlots();

	VillagerSlotWidgets.SetNum(VillagerSlotCount);

	for (int32 i = 0; i < VillagerSlotCount; ++i)
	{
		UInventorySlotWidget* SlotWidget = GetOrCreateVillagerSlotWidget(i);
		if (!SlotWidget) continue;

		FInventorySlot SlotData;
		if (i < Slots.Num()) SlotData = Slots[i];

		SlotWidget->SetSlotData(SlotData, i, false);
		SlotWidget->SetInventorySource(1);
		SlotWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

UInventorySlotWidget* UVillagerManagementWidget::GetOrCreatePlayerSlotWidget(int32 SlotIndex)
{
	if (!PlayerInventoryGrid || SlotIndex < 0 || SlotIndex >= PlayerSlotCount) return nullptr;

	if (PlayerSlotWidgets[SlotIndex] && PlayerSlotWidgets[SlotIndex]->IsValidLowLevel())
		return PlayerSlotWidgets[SlotIndex];

	UInventorySlotWidget* SlotWidget = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		UClass* WidgetClass = SlotWidgetClass ? SlotWidgetClass.Get() : UInventorySlotWidget::StaticClass();
		SlotWidget = CreateWidget<UInventorySlotWidget>(PC, WidgetClass);
	}
	if (!SlotWidget) return nullptr;

	UUniformGridSlot* GridSlot = PlayerInventoryGrid->AddChildToUniformGrid(SlotWidget);
	if (GridSlot)
	{
		GridSlot->SetRow(SlotIndex / GridColumns);
		GridSlot->SetColumn(SlotIndex % GridColumns);
	}

	PlayerSlotWidgets[SlotIndex] = SlotWidget;
	SlotWidget->OnSlotDropped.AddDynamic(this, &UVillagerManagementWidget::HandleSlotDropped);
	return SlotWidget;
}

UInventorySlotWidget* UVillagerManagementWidget::GetOrCreateVillagerSlotWidget(int32 SlotIndex)
{
	if (!VillagerInventoryGrid || SlotIndex < 0 || SlotIndex >= VillagerSlotCount) return nullptr;

	if (VillagerSlotWidgets[SlotIndex] && VillagerSlotWidgets[SlotIndex]->IsValidLowLevel())
		return VillagerSlotWidgets[SlotIndex];

	UInventorySlotWidget* SlotWidget = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		UClass* WidgetClass = SlotWidgetClass ? SlotWidgetClass.Get() : UInventorySlotWidget::StaticClass();
		SlotWidget = CreateWidget<UInventorySlotWidget>(PC, WidgetClass);
	}
	if (!SlotWidget) return nullptr;

	UUniformGridSlot* GridSlot = VillagerInventoryGrid->AddChildToUniformGrid(SlotWidget);
	if (GridSlot)
	{
		GridSlot->SetRow(SlotIndex / GridColumns);
		GridSlot->SetColumn(SlotIndex % GridColumns);
	}

	VillagerSlotWidgets[SlotIndex] = SlotWidget;
	SlotWidget->OnSlotDropped.AddDynamic(this, &UVillagerManagementWidget::HandleSlotDropped);
	return SlotWidget;
}

void UVillagerManagementWidget::HandleSlotDropped(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID)
{
	HandleItemTransfer(SourceSlotIndex, SourceInventoryID, TargetSlotIndex, TargetInventoryID);
}

bool UVillagerManagementWidget::HandleItemTransfer(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID)
{
	if (SourceInventoryID == 0 && TargetInventoryID == 0 && PlayerInventoryComp)
		return PlayerInventoryComp->MoveItemToSlot(SourceSlotIndex, TargetSlotIndex);

	if (SourceInventoryID == 1 && TargetInventoryID == 1 && VillagerInventoryComp)
		return VillagerInventoryComp->MoveItemToSlot(SourceSlotIndex, TargetSlotIndex);

	if (SourceInventoryID == 0 && TargetInventoryID == 1 && PlayerInventoryComp && VillagerInventoryComp)
		return PlayerInventoryComp->TransferStackToOtherInventorySlot(VillagerInventoryComp, SourceSlotIndex, TargetSlotIndex);

	if (SourceInventoryID == 1 && TargetInventoryID == 0 && PlayerInventoryComp && VillagerInventoryComp)
		return VillagerInventoryComp->TransferStackToOtherInventorySlot(PlayerInventoryComp, SourceSlotIndex, TargetSlotIndex);

	return false;
}
