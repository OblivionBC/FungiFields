#include "EffectApplier.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Containers/Map.h"
#include "GameplayEffect.h"

AEffectApplier::AEffectApplier()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEffectApplier::BeginPlay()
{
	Super::BeginPlay();
}

void AEffectApplier::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor)
		return;

	if (IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(Interactor))
	{
		if (UAbilitySystemComponent* TargetASC = TargetInterface->GetAbilitySystemComponent())
		{
			UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("GE_TestDynamic"));

			GE->DurationPolicy = EGameplayEffectDurationType::Instant;

			int32 Idx = GE->Modifiers.AddDefaulted();
			FGameplayModifierInfo& Info = GE->Modifiers[Idx];

			Info.Attribute = Attribute;
			Info.ModifierOp = Operation;
			Info.ModifierMagnitude = FScalableFloat(Value);

			FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
			Context.AddSourceObject(this);
			
			FGameplayEffectSpec Spec(GE, Context, 1.0f);

			TargetASC->ApplyGameplayEffectSpecToSelf(Spec);
		}
	}
}

FText AEffectApplier::GetInteractionText_Implementation()
{
	return FText::FromString("Press E to Interact");
}
