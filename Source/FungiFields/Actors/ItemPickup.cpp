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
	
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	MeshComponent->SetGenerateOverlapEvents(false);
	
	MeshComponent->SetEnableGravity(true);
	MeshComponent->SetSimulatePhysics(true);

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(MeshComponent);
	PickupSphere->SetSphereRadius(PickupRadius);
	
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionObjectType(ECC_WorldDynamic);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);
	
	PickupSphere->SetSimulatePhysics(false);
	PickupSphere->SetEnableGravity(false);
}

void AItemPickup::BeginPlay()
{
	Super::BeginPlay();

	if (PickupSphere)
	{
		PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnOverlapBegin);
	}

	if (PickupSphere && PickupRadius > 0.0f)
	{
		PickupSphere->SetSphereRadius(PickupRadius);
	}

	// WeldTo was removed: PickupSphere is already attached to MeshComponent via SetupAttachment
	// and follows it through USceneComponent parenting. Welding merged their physics bodies,
	// which added an extra physics-body lifecycle during Destroy() that worsened the FName race.
}

void AItemPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

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

	UInventoryComponent* Inventory = PickerUpper->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return;
	}

	if (Inventory->TryAddItem(ItemDataAsset, ItemAmount))
	{
		// Disable collision immediately to block re-entry while the pickup waits for next tick.
		// Calling Destroy() synchronously inside OnComponentBeginOverlap can leave the physics
		// engine mid-overlap-resolution holding a reference to this actor's FName, producing
		// the "block index out of range" assert when it later tries to dereference the freed name.
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(
				FTimerDelegate::CreateWeakLambda(this, [this]() { Destroy(); }));
		}
	}
}

