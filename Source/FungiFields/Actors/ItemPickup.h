#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemPickup.generated.h"

class UItemDataAsset;
class UStaticMeshComponent;
class USphereComponent;

/**
 * Actor that is automatically picked up when a player walks over it.
 * Uses overlap detection to trigger pickup when player enters the pickup radius.
 */
UCLASS()
class FUNGIFIELDS_API AItemPickup : public AActor
{
	GENERATED_BODY()
	
public:
	AItemPickup();

protected:
	virtual void BeginPlay() override;

	/** Called when another actor overlaps with the pickup sphere */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Attempts to add the item to the actor's inventory and destroys the pickup if successful */
	void TryPickupItem(AActor* PickerUpper);

	/** The item data asset that will be added to inventory when picked up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Pickup")
	TObjectPtr<UItemDataAsset> ItemDataAsset;

	/** Amount of items to add when picked up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Pickup", meta = (ClampMin = "1"))
	int32 ItemAmount = 1;

	/** Static mesh component to display the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Pickup")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Sphere component for detecting when player walks over the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Pickup")
	TObjectPtr<USphereComponent> PickupSphere;

	/** Radius of the pickup sphere */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Pickup", meta = (ClampMin = "10.0"))
	float PickupRadius = 50.0f;

	/** If true, enables physics simulation so the item will fall due to gravity (defaults to true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pickup")
	bool bSimulatePhysics = true;
};

