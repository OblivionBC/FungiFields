// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupActor.h"
#include "../Interfaces/PickupInterface.h"

// Sets default values
APickupActor::APickupActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ActorMesh = CreateDefaultSubobject<UStaticMesh>("StaticMesh");

	PickupTriggerSphere = CreateDefaultSubobject<USphereComponent>("PickupTriggerSphere");
	PickupTriggerSphere->SetSphereRadius(50.0f, true);
	PickupTriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupTriggerSphere->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	if (PickupTriggerSphere)
	{
		PickupTriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &APickupActor::TryPickup);
	}
}

// Called every frame
void APickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickupActor::TryPickup(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("Picked up by: %s"), *OtherActor->GetName());
	if (OtherActor && OtherActor->Implements<UPickupInterface>())
	{
		IPickupInterface::Execute_PickupObject(OtherActor);
	}
}

