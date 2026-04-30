#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UVillagerDialogueWidget.generated.h"

class AFarmerVillagerCharacter;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVillagerDialogueClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnManageRequested);

/**
 * Simple dialogue/greeting popup shown when the player interacts with a villager.
 * Displays the villager's name and a greeting, with "Manage" and "Close" buttons.
 * Blueprint child must bind the named widgets.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UVillagerDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Villager Dialogue")
	void SetVillager(AFarmerVillagerCharacter* InVillager);

	UPROPERTY(BlueprintAssignable, Category = "Villager Dialogue")
	FOnVillagerDialogueClosed OnDialogueClosed;

	UPROPERTY(BlueprintAssignable, Category = "Villager Dialogue")
	FOnManageRequested OnManageRequested;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> VillagerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GreetingText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ManageButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

private:
	UFUNCTION()
	void OnManageButtonClicked();

	UFUNCTION()
	void OnCloseButtonClicked();

	UPROPERTY()
	TObjectPtr<AFarmerVillagerCharacter> BoundVillager;
};
