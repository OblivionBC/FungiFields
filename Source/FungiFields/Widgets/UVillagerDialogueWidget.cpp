#include "UVillagerDialogueWidget.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UVillagerDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ManageButton)
	{
		ManageButton->OnClicked.AddDynamic(this, &UVillagerDialogueWidget::OnManageButtonClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UVillagerDialogueWidget::OnCloseButtonClicked);
	}
}

void UVillagerDialogueWidget::NativeDestruct()
{
	if (ManageButton)
	{
		ManageButton->OnClicked.RemoveDynamic(this, &UVillagerDialogueWidget::OnManageButtonClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UVillagerDialogueWidget::OnCloseButtonClicked);
	}

	Super::NativeDestruct();
}

void UVillagerDialogueWidget::SetVillager(AFarmerVillagerCharacter* InVillager)
{
	BoundVillager = InVillager;

	if (!InVillager)
	{
		return;
	}

	if (VillagerNameText)
	{
		VillagerNameText->SetText(InVillager->GetVillagerDisplayName());
	}

	if (GreetingText)
	{
		GreetingText->SetText(FText::Format(
			NSLOCTEXT("VillagerDialogue", "Greeting", "Hello! I'm working as a {0}."),
			FText::FromString(UEnum::GetDisplayValueAsText(InVillager->GetAssignedRole()).ToString())
		));
	}
}

void UVillagerDialogueWidget::OnManageButtonClicked()
{
	OnManageRequested.Broadcast();
	RemoveFromParent();
}

void UVillagerDialogueWidget::OnCloseButtonClicked()
{
	OnDialogueClosed.Broadcast();
	RemoveFromParent();
}
