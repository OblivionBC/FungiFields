// Fill out your copyright notice in the Description page of Project Settings.


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
	LevelAttributeSet = NewObject<ULevelAttributeSet>(this);
	if (UAbilitySystemComponent * ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>())
	{
		if (LevelAttributeSet)
		{
			ASC->AddAttributeSetSubobject(LevelAttributeSet);
			
			if (InitialStatsGE) 
		  {
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InitialStatsGE, 1, Context);

				if (SpecHandle.IsValid())
				{
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Level")), 1);
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.MaxLevel")), 100);
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.XP")), 0);
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.MaxXP")), 100);

					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
		  }
		}
		BindDelegates(ASC);
	}
}

float ULevelComponent::GetXP()
{
	if (LevelAttributeSet)
	{
		return LevelAttributeSet->GetXP();
	}
	return 0;
}

float ULevelComponent::GetMaxXP()
{
	if (LevelAttributeSet)
	{
		return LevelAttributeSet->GetMaxXP();
	}
	return 0;
}

void ULevelComponent::BindDelegates(UAbilitySystemComponent * ASC)
{
	ASC->GetGameplayAttributeValueChangeDelegate(LevelAttributeSet->GetLevelAttribute())
			.AddUObject(this, &ULevelComponent::CheckLevelUp);
}

void ULevelComponent::CheckLevelUp(const FOnAttributeChangeData& Data)
{
	if (!LevelAttributeSet)
	{
		return;
	}
	float MaxXP = LevelAttributeSet->GetMaxXP();
	float CurrXP = LevelAttributeSet->GetXP();
	int Levels = 0;
	while (CurrXP >=  MaxXP)
	{
		CurrXP -= MaxXP;
		LevelAttributeSet->SetXP(CurrXP);
		MaxXP = LevelAttributeSet->GetMaxXP();
		Levels++;
	}
	LevelAttributeSet->SetLevel(LevelAttributeSet->GetLevel() + Levels);
}



