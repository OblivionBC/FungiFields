#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "EconomyAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Attribute Set containing economy-related attributes.
 * Manages currency values such as Gold.
 */
UCLASS()
class FUNGIFIELDS_API UEconomyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UEconomyAttributeSet();

	ATTRIBUTE_ACCESSORS(UEconomyAttributeSet, Gold)
	ATTRIBUTE_ACCESSORS(UEconomyAttributeSet, MaxGold)

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Economy Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Gold;

	UPROPERTY(BlueprintReadOnly, Category = "Economy Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxGold;
};

