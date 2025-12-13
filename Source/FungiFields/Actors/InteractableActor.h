#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/InteractableInterface.h"
#include "InteractableActor.generated.h"

class UWidgetComponent;
class UInteractionWidget;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class AInteractableActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AInteractableActor();

	virtual void Tick(float DeltaSeconds) override;

	// Interaction interface
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;

	// ITooltipProvider implementation
	virtual FText GetTooltipText_Implementation() const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProximityEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						  bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProximityExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdateWidgetTransform() const;
	void BillboardToCamera();

	UInteractionWidget* GetInteractionWidget() const;

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Proximity;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* Widget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UInteractionWidget> WidgetClass;

	UPROPERTY(EditAnywhere)
	float WidgetHeightOffset = 100.f;

	UPROPERTY(EditAnywhere)
	bool bFacePlayerCamera = true;
};
