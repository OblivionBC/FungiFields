#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "FungiFields/Interfaces/IInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AEffectApplier.generated.h"

struct FGameplayAttribute;

UCLASS()
class FUNGIFIELDS_API AEffectApplier : public AActor,  public IInteractableInterface
{
	GENERATED_BODY()

public:
	AEffectApplier();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;

	virtual FText GetTooltipText_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	TEnumAsByte<EGameplayModOp::Type> Operation = EGameplayModOp::Additive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;
};