#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "InteractableItemPickup.generated.h"

class UItemDataAsset;

/**
 * Actor that can be picked up by interacting with it (pressing E).
 * Inherits from InteractableActor to use the existing interaction system with widgets.
 */
UCLASS()
class FUNGIFIELDS_API AInteractableItemPickup : public AInteractableActor
{
	GENERATED_BODY()
	
public:
	AInteractableItemPickup();

	// Override interaction implementation
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;
	virtual FText GetTooltipText_Implementation() const override;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	/** Attempts to add the item to the actor's inventory and destroys the pickup if successful */
	void TryPickupItem(AActor* PickerUpper);

	/** The item data asset that will be added to inventory when picked up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Pickup")
	TObjectPtr<UItemDataAsset> ItemDataAsset;

	/** Amount of items to add when picked up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Pickup", meta = (ClampMin = "1"))
	int32 ItemAmount = 1;

	/** If true, enables physics simulation so the item will fall due to gravity (defaults to true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pickup")
	bool bSimulatePhysics = true;
};
