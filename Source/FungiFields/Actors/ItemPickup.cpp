#include "ItemPickup.h"
#include "../Data/UItemDataAsset.h"
#include "../Components/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AItemPickup::AItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	
	// Set up mesh collision: collide with world (everything) but ignore pawns
	// Enable physics so gravity works
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	MeshComponent->SetGenerateOverlapEvents(false);
	
	MeshComponent->SetEnableGravity(true);
	MeshComponent->SetSimulatePhysics(true);

	// Create sphere component for pickup detection - attached to mesh so it moves with the physics body
	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(MeshComponent);
	PickupSphere->SetSphereRadius(PickupRadius);
	
	// Set up sphere collision: overlap only with pawns for pickup detection
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionObjectType(ECC_WorldDynamic);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);
	
	// Ensure sphere doesn't simulate physics and follows the parent physics body
	PickupSphere->SetSimulatePhysics(false);
	PickupSphere->SetEnableGravity(false);
}

void AItemPickup::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap event
	if (PickupSphere)
	{
		PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnOverlapBegin);
	}

	// Update sphere radius if it was changed in editor
	if (PickupSphere && PickupRadius > 0.0f)
	{
		PickupSphere->SetSphereRadius(PickupRadius);
	}

	if (MeshComponent && PickupSphere)
	{
		PickupSphere->WeldTo(MeshComponent);
	}
}

void AItemPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// Only process pickup for pawns (players and NPCs)
	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (!OverlappingPawn)
	{
		return;
	}

	TryPickupItem(OtherActor);
}

void AItemPickup::TryPickupItem(AActor* PickerUpper)
{
	if (!PickerUpper || !ItemDataAsset)
	{
		return;
	}

	// Check if the actor has an inventory component
	UInventoryComponent* Inventory = PickerUpper->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return;
	}
	
	// If successfully added, destroy the actor
	if (Inventory->TryAddItem(ItemDataAsset, ItemAmount))
	{
		Destroy();
	}
}

