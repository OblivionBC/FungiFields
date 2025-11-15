#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Components/ProgressBar.h"
#include "PlayerHUDWidget.generated.h"

class UStatsBarWidget;
class UInventorySlotsWidget;
class UTextBlock;
class AFungiFieldsCharacter;
class UAbilitySystemComponent;
class UEconomyAttributeSet;
class ULevelAttributeSet;

/**
 * Main HUD widget that serves as the composition root.
 * Assembles smaller dedicated widgets and binds to data sources.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStatsBarWidget> StatsBarContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> XPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventorySlotsWidget> InventoryDisplay;

	virtual void NativeOnInitialized() override;

private:
	void OnGoldUpdated(const FOnAttributeChangeData& Data);
	void OnXPUpdated(const FOnAttributeChangeData& Data);
	void OnLevelUpdated(const FOnAttributeChangeData& Data);

	AFungiFieldsCharacter* GetPlayerCharacter() const;

	void BindToAttributeDelegates();

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;
};