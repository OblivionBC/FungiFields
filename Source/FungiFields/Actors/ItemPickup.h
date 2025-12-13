#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/InteractableInterface.h"
#include "ItemPickup.generated.h"

class UItemDataAsset;
class UStaticMeshComponent;
class USphereComponent;

/**
 * Actor that can be picked up and added to player inventory.
 * Implements IInteractableInterface to work with the existing interaction system.
 * Uses Strategy Pattern - the interaction behavior is defined by the interface.
 */
UCLASS()
class FUNGIFIELDS_API AItemPickup : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:
	AItemPickup();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;

	// ITooltipProvider implementation
	virtual FText GetTooltipText_Implementation() const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Pickup")
	TObjectPtr<UItemDataAsset> ItemDataAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Pickup")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Pickup")
	TObjectPtr<USphereComponent> CollisionSphere;
};

