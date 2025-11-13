#include "EffectApplier.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"


AEffectApplier::AEffectApplier()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEffectApplier::BeginPlay()
{
	Super::BeginPlay();
}

void AEffectApplier::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEffectApplier::Interact_Implementation(AActor* Interactor)
{
	ApplyEffect(EffectClassToApply, Interactor);
}

FText AEffectApplier::GetInteractionText_Implementation()
{
	return FText::FromString("Press E to Interact");
}

void AEffectApplier::ApplyEffect_Implementation(TSubclassOf<class UGameplayEffect> EffectClass, AActor* Interactor)
{
	if (IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(Interactor))
	{
		if (UAbilitySystemComponent* TargetASC = TargetInterface->GetAbilitySystemComponent())
		{
			FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(
				EffectClass,
				1.0f,
				Context);
			FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(
				*SpecHandle.Data.Get());
		}
	}
}