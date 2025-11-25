#include "StatsBarWidget.h"
#include "../Characters/FungiFieldsCharacter.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "../Attributes/CharacterAttributeSet.h"
#include "Components/Widget.h"
#include "Components/CanvasPanelSlot.h"

void UStatsBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HealthBar)
	{
		BaseHealthBarWidth = HealthBar->GetDesiredSize().X;
		if (BaseHealthBarWidth <= 0.0f)
		{
			BaseHealthBarWidth = BaseBarWidth;
		}
	}

	if (StaminaBar)
	{
		BaseStaminaBarWidth = StaminaBar->GetDesiredSize().X;
		if (BaseStaminaBarWidth <= 0.0f)
		{
			BaseStaminaBarWidth = BaseBarWidth;
		}
	}

	if (MagicBar)
	{
		BaseMagicBarWidth = MagicBar->GetDesiredSize().X;
		if (BaseMagicBarWidth <= 0.0f)
		{
			BaseMagicBarWidth = BaseBarWidth / 2;
		}
	}

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
	if (!ASC || !PlayerCharacter->CharacterAttributeSet)
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

		ASC->GetGameplayAttributeValueChangeDelegate(CharacterAttributeSet->GetMaxHealthAttribute())
			.AddUObject(this, &UStatsBarWidget::OnMaxHealthUpdated);

		ASC->GetGameplayAttributeValueChangeDelegate(CharacterAttributeSet->GetMaxStaminaAttribute())
			.AddUObject(this, &UStatsBarWidget::OnMaxStaminaUpdated);

		ASC->GetGameplayAttributeValueChangeDelegate(CharacterAttributeSet->GetMaxMagicAttribute())
			.AddUObject(this, &UStatsBarWidget::OnMaxMagicUpdated);

		if (HealthBar)
		{
			const float CurrentHealth = CharacterAttributeSet->GetHealth();
			const float MaxHealth = CharacterAttributeSet->GetMaxHealth();
			const float HealthPercent = MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
			HealthBar->SetPercent(HealthPercent);
			UpdateBarWidth(HealthBar, MaxHealth, BaseMaxHealth);
		}

		if (StaminaBar)
		{
			const float CurrentStamina = CharacterAttributeSet->GetStamina();
			const float MaxStamina = CharacterAttributeSet->GetMaxStamina();
			const float StaminaPercent = MaxStamina > 0.0f ? (CurrentStamina / MaxStamina) : 0.0f;
			StaminaBar->SetPercent(StaminaPercent);
			UpdateBarWidth(StaminaBar, MaxStamina, BaseMaxStamina);
		}

		if (MagicBar)
		{
			const float CurrentMagic = CharacterAttributeSet->GetMagic();
			const float MaxMagic = CharacterAttributeSet->GetMaxMagic();
			const float MagicPercent = MaxMagic > 0.0f ? (CurrentMagic / MaxMagic) : 0.0f;
			MagicBar->SetPercent(MagicPercent);
			UpdateBarWidth(MagicBar, MaxMagic, BaseMaxMagic);
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

void UStatsBarWidget::OnMaxHealthUpdated(const FOnAttributeChangeData& Data)
{
	if (HealthBar)
	{
		UpdateBarWidth(HealthBar, Data.NewValue, BaseMaxHealth);
		AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
		if (PlayerCharacter && PlayerCharacter->CharacterAttributeSet)
		{
			const float CurrentHealth = PlayerCharacter->CharacterAttributeSet->GetHealth();
			const float HealthPercent = Data.NewValue > 0.0f ? (CurrentHealth / Data.NewValue) : 0.0f;
			HealthBar->SetPercent(HealthPercent);
		}
	}
}

void UStatsBarWidget::OnMaxStaminaUpdated(const FOnAttributeChangeData& Data)
{
	if (StaminaBar)
	{
		UpdateBarWidth(StaminaBar, Data.NewValue, BaseMaxStamina);
		AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
		if (PlayerCharacter && PlayerCharacter->CharacterAttributeSet)
		{
			const float CurrentStamina = PlayerCharacter->CharacterAttributeSet->GetStamina();
			const float StaminaPercent = Data.NewValue > 0.0f ? (CurrentStamina / Data.NewValue) : 0.0f;
			StaminaBar->SetPercent(StaminaPercent);
		}
	}
}

void UStatsBarWidget::OnMaxMagicUpdated(const FOnAttributeChangeData& Data)
{
	if (MagicBar)
	{
		UpdateBarWidth(MagicBar, Data.NewValue, BaseMaxMagic);
		AFungiFieldsCharacter* PlayerCharacter = GetPlayerCharacter();
		if (PlayerCharacter && PlayerCharacter->CharacterAttributeSet)
		{
			const float CurrentMagic = PlayerCharacter->CharacterAttributeSet->GetMagic();
			const float MagicPercent = Data.NewValue > 0.0f ? (CurrentMagic / Data.NewValue) : 0.0f;
			MagicBar->SetPercent(MagicPercent);
		}
	}
}

void UStatsBarWidget::UpdateBarWidth(UProgressBar* Bar, float CurrentMax, float BaseMax)
{
	if (!Bar || BaseMax <= 0.0f)
	{
		return;
	}

	float BaseWidth = 0.0f;
	if (Bar == HealthBar)
	{
		BaseWidth = BaseHealthBarWidth;
	}
	else if (Bar == StaminaBar)
	{
		BaseWidth = BaseStaminaBarWidth;
	}
	else if (Bar == MagicBar)
	{
		BaseWidth = BaseMagicBarWidth;
	}

	if (BaseWidth <= 0.0f)
	{
		return;
	}

	float ScaleFactor = CurrentMax / BaseMax;
	ScaleFactor = FMath::Clamp(ScaleFactor, MinScaleFactor, MaxScaleFactor);
	float NewWidth = BaseWidth * ScaleFactor;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Bar->Slot))
	{
		FVector2D CurrentSize = CanvasSlot->GetSize();
		CanvasSlot->SetSize(FVector2D(NewWidth, CurrentSize.Y));
		CanvasSlot->SetAlignment(FVector2D(0.0f, 0.5f));
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.5f));
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