#include "UConsumableDataAsset.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "../Attributes/CharacterAttributeSet.h"

UConsumableDataAsset::UConsumableDataAsset()
{
	MaxStackSize = 10;
}

bool UConsumableDataAsset::UseItem_Implementation(AActor* User)
{
	if (!User) return false;

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(User);
	if (!ASCInterface) return false;

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC) return false;

	bool bAppliedAny = false;

	if (HealthRestoreAmount > 0.f)
	{
		ASC->ApplyModToAttribute(UCharacterAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, HealthRestoreAmount);
		bAppliedAny = true;
	}

	if (StaminaRestoreAmount > 0.f)
	{
		ASC->ApplyModToAttribute(UCharacterAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, StaminaRestoreAmount);
		bAppliedAny = true;
	}

	return bAppliedAny;
}

bool UConsumableDataAsset::CanUseItem_Implementation(const AActor* User) const
{
	return User != nullptr;
}
