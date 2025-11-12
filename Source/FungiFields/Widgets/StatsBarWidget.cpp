#include "StatsBarWidget.h"
#include "../Characters/FungiFieldsCharacter.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "../Attributes/CharacterAttributeSet.h"

void UStatsBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToPlayerComponentDelegates();
}

void UStatsBarWidget::BindToPlayerComponentDelegates()
{
	AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	CachedAbilitySystemComponent = ASC;

	if (const UCharacterAttributeSet* CharacterAttributeSet = PlayerCharacter->CharacterAttributeSet)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(CharacterAttributeSet->GetHealthAttribute())
			.AddUObject(this, &UStatsBarWidget::OnHealthUpdated);

		ASC->GetGameplayAttributeValueChangeDelegate(CharacterAttributeSet->GetStaminaAttribute())
			.AddUObject(this, &UStatsBarWidget::OnStaminaUpdated);

		ASC->GetGameplayAttributeValueChangeDelegate(CharacterAttributeSet->GetMagicAttribute())
			.AddUObject(this, &UStatsBarWidget::OnMagicUpdated);

		if (HealthBar)
		{
			const float CurrentHealth = CharacterAttributeSet->GetHealth();
			const float MaxHealth = CharacterAttributeSet->GetMaxHealth();
			const float HealthPercent = MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
			HealthBar->SetPercent(HealthPercent);
		}

		if (StaminaBar)
		{
			const float CurrentStamina = CharacterAttributeSet->GetStamina();
			const float MaxStamina = CharacterAttributeSet->GetMaxStamina();
			const float StaminaPercent = MaxStamina > 0.0f ? (CurrentStamina / MaxStamina) : 0.0f;
			StaminaBar->SetPercent(StaminaPercent);
		}

		if (MagicBar)
		{
			const float CurrentMagic = CharacterAttributeSet->GetMagic();
			const float MaxMagic = CharacterAttributeSet->GetMaxMagic();
			const float MagicPercent = MaxMagic > 0.0f ? (CurrentMagic / MaxMagic) : 0.0f;
			MagicBar->SetPercent(MagicPercent);
		}
	}
}

void UStatsBarWidget::OnHealthUpdated(const FOnAttributeChangeData& Data)
{
	if (HealthBar)
	{
		AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
		if (PlayerCharacter && PlayerCharacter->CharacterAttributeSet)
		{
			const float MaxHealth = PlayerCharacter->CharacterAttributeSet->GetMaxHealth();
			const float HealthPercent = MaxHealth > 0.0f ? (Data.NewValue / MaxHealth) : 0.0f;
			HealthBar->SetPercent(HealthPercent);
		}
	}
}

void UStatsBarWidget::OnStaminaUpdated(const FOnAttributeChangeData& Data)
{
	if (StaminaBar)
	{
		AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
		if (PlayerCharacter && PlayerCharacter->CharacterAttributeSet)
		{
			const float MaxStamina = PlayerCharacter->CharacterAttributeSet->GetMaxStamina();
			const float StaminaPercent = MaxStamina > 0.0f ? (Data.NewValue / MaxStamina) : 0.0f;
			StaminaBar->SetPercent(StaminaPercent);
		}
	}
}

void UStatsBarWidget::OnMagicUpdated(const FOnAttributeChangeData& Data)
{
	if (MagicBar)
	{
		AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
		if (PlayerCharacter && PlayerCharacter->CharacterAttributeSet)
		{
			const float MaxMagic = PlayerCharacter->CharacterAttributeSet->GetMaxMagic();
			const float MagicPercent = MaxMagic > 0.0f ? (Data.NewValue / MaxMagic) : 0.0f;
			MagicBar->SetPercent(MagicPercent);
		}
	}
}

AFungiFieldsCharacter* UStatsBarWidget::GetPlayerCharacter() const
{
	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		return Cast<AFungiFieldsCharacter>(OwningPawn);
	}
	return nullptr;
}