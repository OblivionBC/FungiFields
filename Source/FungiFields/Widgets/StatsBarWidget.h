#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "StatsBarWidget.generated.h"

class UProgressBar;
class AFungiFieldsCharacter;
class UAbilitySystemComponent;
class UCharacterAttributeSet;

/**
 * Widget dedicated to displaying Health, Stamina, and Magic progress bars.
 * Binds to CharacterAttributeSet via Gameplay Ability System delegates.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UStatsBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> MagicBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats Bar Scaling", meta = (ClampMin = "1.0"))
	float BaseMaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats Bar Scaling", meta = (ClampMin = "1.0"))
	float BaseMaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats Bar Scaling", meta = (ClampMin = "1.0"))
	float BaseMaxMagic = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats Bar Scaling", meta = (ClampMin = "1.0"))
	float BaseBarWidth = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats Bar Scaling", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinScaleFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats Bar Scaling", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float MaxScaleFactor = 3.0f;

	virtual void NativeConstruct() override;

private:
	void OnHealthUpdated(const FOnAttributeChangeData& Data);

	void OnStaminaUpdated(const FOnAttributeChangeData& Data);

	void OnMagicUpdated(const FOnAttributeChangeData& Data);

	void OnMaxHealthUpdated(const FOnAttributeChangeData& Data);

	void OnMaxStaminaUpdated(const FOnAttributeChangeData& Data);

	void OnMaxMagicUpdated(const FOnAttributeChangeData& Data);

	void UpdateBarWidth(UProgressBar* Bar, float CurrentMax, float BaseMax);

	void BindToPlayerComponentDelegates();

	AFungiFieldsCharacter* GetPlayerCharacter() const;

	float BaseHealthBarWidth = 0.0f;
	float BaseStaminaBarWidth = 0.0f;
	float BaseMagicBarWidth = 0.0f;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;
};