#include "InteractableItemPickup.h"
#include "../Data/UItemDataAsset.h"
#include "../Components/InventoryComponent.h"
#include "Components/WidgetComponent.h"

AInteractableItemPickup::AInteractableItemPickup()
{
}

void AInteractableItemPickup::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionObjectType(ECC_WorldDynamic);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Mesh->SetGenerateOverlapEvents(false);
		
		Mesh->SetEnableGravity(true);
		
		FTransform MeshTransform = Mesh->GetComponentTransform();
		RootComponent = Mesh;
		Mesh->SetWorldTransform(MeshTransform);
	}
}

void AInteractableItemPickup::BeginPlay()
{
	Super::BeginPlay();

	if (Mesh)
	{
		Mesh->SetSimulatePhysics(bSimulatePhysics);
	}
}

void AInteractableItemPickup::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	TryPickupItem(Interactor);
}

FText AInteractableItemPickup::GetInteractionText_Implementation()
{
	if (ItemDataAsset)
	{
		return FText::Format(NSLOCTEXT("InteractableItemPickup", "PickupText", "Pick up {0}"), ItemDataAsset->ItemName);
	}
	return NSLOCTEXT("InteractableItemPickup", "PickupTextDefault", "Pick up Item");
}

FText AInteractableItemPickup::GetTooltipText_Implementation() const
{
	if (ItemDataAsset)
	{
		return FText::Format(NSLOCTEXT("InteractableItemPickup", "PickupText", "Pick up {0}"), ItemDataAsset->ItemName);
	}
	return NSLOCTEXT("InteractableItemPickup", "PickupTextDefault", "Pick up Item");
}

void AInteractableItemPickup::TryPickupItem(AActor* PickerUpper)
{
	if (!PickerUpper || !ItemDataAsset)
	{
		return;
	}

	UInventoryComponent* Inventory = PickerUpper->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return;
	}

	bool bSuccess = Inventory->TryAddItem(ItemDataAsset, ItemAmount);

	if (bSuccess)
	{
		Destroy();
	}
}




