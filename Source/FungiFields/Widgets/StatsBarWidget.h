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

	virtual void NativeConstruct() override;

private:
	void OnHealthUpdated(const FOnAttributeChangeData& Data);

	void OnStaminaUpdated(const FOnAttributeChangeData& Data);

	void OnMagicUpdated(const FOnAttributeChangeData& Data);

	void BindToPlayerComponentDelegates();

	AFungiFieldsCharacter* GetPlayerCharacter() const;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;
};