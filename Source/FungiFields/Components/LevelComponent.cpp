#include "LevelComponent.h"

#include "AbilitySystemInterface.h"
#include "FungiFields/Attributes/LevelAttributeSet.h"


ULevelComponent::ULevelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULevelComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	if (!Owner)
		return;

	if (UAbilitySystemComponent* AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>())
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

void ULevelComponent::CheckLevelUp(const FOnAttributeChangeData& Data)
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

void ULevelComponent::LevelUp(int Levels)
{
	ASC->ApplyModToAttribute(
		ULevelAttributeSet::GetLevelAttribute(),
		EGameplayModOp::Additive,
		Levels
	);

	OnLevelUp.Broadcast(static_cast<int32>(LevelAttributeSet->GetLevel()));
}

void ULevelComponent::LowerByMaxXP(int MaxXP)
{
	ASC->ApplyModToAttribute(
		ULevelAttributeSet::GetXPAttribute(),
		EGameplayModOp::Additive,
		-MaxXP
	);
}

