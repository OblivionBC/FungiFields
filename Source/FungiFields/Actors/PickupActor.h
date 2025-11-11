// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "FungiFields/Interfaces/PickupInterface.h"
#include "GameFramework/Actor.h"
#include "PickupActor.generated.h"

UCLASS()
class FUNGIFIELDS_API APickupActor : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	APickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	UStaticMeshComponent* ActorMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	USphereComponent* PickupTriggerSphere;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void TryPickup(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
