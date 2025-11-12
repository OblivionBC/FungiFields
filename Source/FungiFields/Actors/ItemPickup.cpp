#include "ItemPickup.h"
#include "../Data/UItemDataAsset.h"
#include "../Components/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AItemPickup::AItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetSphereRadius(50.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionSphere);
}

void AItemPickup::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor || !ItemDataAsset)
	{
		return;
	}

	UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();

	if (Inventory)
	{
		bool bSuccess = Inventory->TryAddItem(ItemDataAsset, 1);

		if (bSuccess)
		{
			Destroy();
			return;
		}
		
		// Log failure for debugging
		UE_LOG(LogTemp, Warning, TEXT("ItemPickup::Interact_Implementation: Failed to add item. Interactor: %s, ItemDataAsset: %s"), 
			*Interactor->GetName(), *ItemDataAsset->GetName());
	}
}

FText AItemPickup::GetInteractionText_Implementation()
{
	if (ItemDataAsset)
	{
		return FText::Format(NSLOCTEXT("ItemPickup", "PickupText", "Pick up {0}"), ItemDataAsset->ItemName);
	}
	return NSLOCTEXT("ItemPickup", "PickupTextDefault", "Pick up Item");
}

