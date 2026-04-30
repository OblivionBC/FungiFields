#include "UVillagerManagementWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "../Data/UToolDataAsset.h"
#include "../Data/USeedDataAsset.h"

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
}

void UVillagerManagementWidget::NativeDestruct()
{
	if (BoundVillager)
	{
		BoundVillager->OnRoleChanged.RemoveDynamic(this, &UVillagerManagementWidget::HandleRoleChanged);

		if (UInventoryComponent* InvComp = BoundVillager->GetInventoryComponent())
		{
			InvComp->OnInventoryChanged.RemoveDynamic(this, &UVillagerManagementWidget::HandleInventoryChanged);
		}
	}

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
		InvComp->OnInventoryChanged.AddDynamic(this, &UVillagerManagementWidget::HandleInventoryChanged);
	}

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
}
