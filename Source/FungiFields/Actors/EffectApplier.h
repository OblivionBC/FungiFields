#pragma once

#include "CoreMinimal.h"
#include "FungiFields/Interfaces/InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "EffectApplier.generated.h"

UCLASS()
class FUNGIFIELDS_API AEffectApplier : public AActor,  public IInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEffectApplier();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame

	UPROPERTY(EditAnywhere, Category = "Effect")
	TSubclassOf<class UGameplayEffect> EffectClassToApply;
	
	virtual void Tick(float DeltaTime) override;

	// Interaction interface
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ApplyEffect(TSubclassOf<class UGameplayEffect> EffectClass, AActor* Interactor);
};