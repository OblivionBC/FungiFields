#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "FungiFields/Interfaces/InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "EffectApplier.generated.h"

struct FGameplayAttribute;

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

	// Interaction interface
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	TEnumAsByte<EGameplayModOp::Type> Operation = EGameplayModOp::Additive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;
};