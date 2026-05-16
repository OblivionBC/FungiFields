#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/IInteractableInterface.h"
#include "../Components/UShopComponent.h"
#include "AShopActor.generated.h"

class UStaticMeshComponent;

/**
 * Standalone placeable shop actor — a thin wrapper around UShopComponent.
 * All buy/sell logic, stock management, and widget handling lives in UShopComponent.
 * You can also grant shop behaviour to any other actor (e.g. a villager) by attaching
 * UShopComponent to it directly without using this actor class.
 */
UCLASS()
class FUNGIFIELDS_API AShopActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AShopActor();

	// ~IInteractableInterface
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;
	virtual FText GetTooltipText_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "Shop")
	UShopComponent* GetShopComponent() const { return ShopComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UShopComponent> ShopComponent;
};
