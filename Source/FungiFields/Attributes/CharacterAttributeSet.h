#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CharacterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Attribute Set containing character combat and resource attributes.
 * Includes Health, Stamina, and Magic with their respective maximum values.
 */
UCLASS()
class FUNGIFIELDS_API UCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCharacterAttributeSet();

	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxHealth)

	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Stamina)
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxStamina)

	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Magic)
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxMagic)

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Stamina;

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxStamina;

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Magic;

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxMagic;
};
