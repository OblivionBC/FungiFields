#pragma once

#include "CoreMinimal.h"
#include "AInteractableActor.h"
#include "AChestActor.generated.h"

class UChestInventoryComponent;
class UChestWidget;
class AFungiFieldsCharacter;

/**
 * Actor representing a chest that can be placed and interacted with.
 * Contains an inventory that players can access.
 */
UCLASS()
class FUNGIFIELDS_API AChestActor : public AInteractableActor
{
	GENERATED_BODY()

public:
	AChestActor();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Chest")
	UChestInventoryComponent* GetChestInventoryComponent() const { return ChestInventoryComponent; }

	/** Delegate broadcast when chest widget is closed */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChestClosed);
	UPROPERTY(BlueprintAssignable, Category = "Chest")
	FOnChestClosed OnChestClosed;

protected:
	virtual void BeginPlay() override;

	/** Chest inventory component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest")
	UChestInventoryComponent* ChestInventoryComponent;

	/** Widget class to display when chest is opened */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
	TSubclassOf<UChestWidget> ChestWidgetClass;

private:
	/** Currently open chest widget for this player (client-side) */
	UPROPERTY()
	TObjectPtr<UChestWidget> ChestWidgetInstance;

	/** Player who opened the chest (for input-block pop on close). */
	TWeakObjectPtr<AFungiFieldsCharacter> ChestOpenerCharacter;

	void OpenChestWidgetForPlayer(AActor* Interactor);
	void CloseChestWidget();

	UFUNCTION()
	void OnChestWidgetClosed();
};
