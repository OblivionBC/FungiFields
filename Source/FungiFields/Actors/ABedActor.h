#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/InteractableInterface.h"
#include "ABedActor.generated.h"

class UStaticMeshComponent;

/**
 * Interactable bed that lets the player skip to morning.
 * On interact, fast-forwards UDayNightSubsystem to DayStartHour.
 */
UCLASS()
class FUNGIFIELDS_API ABedActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ABedActor();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;
	virtual FText GetTooltipText_Implementation() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bed")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bed")
	FText BedName = FText::FromString(TEXT("Bed"));
};
