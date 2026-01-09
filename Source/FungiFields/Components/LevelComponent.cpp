#include "LevelComponent.h"

#include "AbilitySystemInterface.h"
#include "FungiFields/Attributes/LevelAttributeSet.h"


// Sets default values for this component's properties
ULevelComponent::ULevelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void ULevelComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UAbilitySystemComponent * AbilitySystemComponent = GetOwner()->FindComponentByClass<UAbilitySystemComponent>())
	{
		this->ASC = AbilitySystemComponent;
		LevelAttributeSet = AbilitySystemComponent->GetSet<ULevelAttributeSet>();
		if (!LevelAttributeSet)
		{
			UE_LOG(LogTemp, Error, TEXT("LevelComponent: LevelAttributeSet missing!"));
			return;
		}
		BindDelegates();
	}
}

float ULevelComponent::GetXP() const
{
	if (LevelAttributeSet)
	{
		return LevelAttributeSet->GetXP();
	}
	return 0;
}

float ULevelComponent::GetMaxXP() const
{
	if (LevelAttributeSet)
	{
		return LevelAttributeSet->GetMaxXP();
	}
	return 0;
}

void ULevelComponent::BindDelegates()
{
	ASC->GetGameplayAttributeValueChangeDelegate(LevelAttributeSet->GetXPAttribute())
			.AddUObject(this, &ULevelComponent::CheckLevelUp);
}

void ULevelComponent::CheckLevelUp(const FOnAttributeChangeData& Data) const
{
	if (!LevelAttributeSet || !ASC)
	{
		return;
	}
	float MaxXP = LevelAttributeSet->GetMaxXP();
	float CurrXP = LevelAttributeSet->GetXP();
	int Levels = 0;
	
	while (CurrXP >=  MaxXP)
	{
		CurrXP -= MaxXP;
		LowerByMaxXP(MaxXP);
		MaxXP = LevelAttributeSet->GetMaxXP();
		Levels++;
	}
	if (Levels > 0)
	{
		LevelUp(Levels);
	}
}

void ULevelComponent::LevelUp(int Levels) const
{
	ASC->ApplyModToAttribute(
		ULevelAttributeSet::GetLevelAttribute(),
		EGameplayModOp::Additive,
		Levels
	);
}

void ULevelComponent::LowerByMaxXP(int MaxXP) const
{
	ASC->ApplyModToAttribute(
		ULevelAttributeSet::GetXPAttribute(),
		EGameplayModOp::Additive,
		-MaxXP
	);
}



