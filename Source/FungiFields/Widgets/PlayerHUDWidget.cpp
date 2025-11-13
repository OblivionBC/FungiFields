#include "PlayerHUDWidget.h"
#include "../Characters/FungiFieldsCharacter.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "../Attributes/EconomyAttributeSet.h"
#include "../Attributes/LevelAttributeSet.h"

void UPlayerHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindToAttributeDelegates();
}

void UPlayerHUDWidget::BindToAttributeDelegates()
{
	AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
	if (!ASC || !PlayerCharacter->EconomyAttributeSet || !PlayerCharacter->LevelAttributeSet)
	{
		return;
	}

	CachedAbilitySystemComponent = ASC;

	if (const UEconomyAttributeSet* EconomyAttributeSet = PlayerCharacter->EconomyAttributeSet)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(EconomyAttributeSet->GetGoldAttribute())
			.AddUObject(this, &UPlayerHUDWidget::OnGoldUpdated);
		
		const float CurrentGold = EconomyAttributeSet->GetGold();
		if (GoldText)
		{
			GoldText->SetText(FText::AsNumber(FMath::FloorToInt(CurrentGold)));
		}
	}

	if (const ULevelAttributeSet* LevelAttributeSet = PlayerCharacter->LevelAttributeSet)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(LevelAttributeSet->GetLevelAttribute())
			.AddUObject(this, &UPlayerHUDWidget::OnLevelUpdated);
		
		const float CurrentLevel = LevelAttributeSet->GetLevel();
		if (LevelText)
		{
			LevelText->SetText(FText::AsNumber(FMath::FloorToInt(CurrentLevel)));
		}
	}
}

void UPlayerHUDWidget::OnGoldUpdated(const FOnAttributeChangeData& Data)
{
	if (GoldText)
	{
		const int32 NewGoldAmount = FMath::FloorToInt(Data.NewValue);
		GoldText->SetText(FText::AsNumber(NewGoldAmount));
	}
}

void UPlayerHUDWidget::OnLevelUpdated(const FOnAttributeChangeData& Data)
{
	if (LevelText)
	{
		const int32 NewLevel = FMath::FloorToInt(Data.NewValue);
		LevelText->SetText(FText::AsNumber(NewLevel));
	}
}

AFungiFieldsCharacter* UPlayerHUDWidget::GetPlayerCharacter() const
{
	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		return Cast<AFungiFieldsCharacter>(OwningPawn);
	}
	return nullptr;
}