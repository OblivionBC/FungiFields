#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "LevelAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Attribute Set containing leveling and progression attributes.
 * Manages character level, experience points, and their maximum values.
 */
UCLASS()
class FUNGIFIELDS_API ULevelAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	ULevelAttributeSet();

	ATTRIBUTE_ACCESSORS(ULevelAttributeSet, Level)
	ATTRIBUTE_ACCESSORS(ULevelAttributeSet, MaxLevel)

	ATTRIBUTE_ACCESSORS(ULevelAttributeSet, XP)
	ATTRIBUTE_ACCESSORS(ULevelAttributeSet, MaxXP)

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Level Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Level;

	UPROPERTY(BlueprintReadOnly, Category = "Level Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Level Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData XP;

	UPROPERTY(BlueprintReadOnly, Category = "Level Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxXP;
};

