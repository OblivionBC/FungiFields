#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../ENUM/EFarmerRole.h"
#include "UVillagerManagementWidget.generated.h"

class AFarmerVillagerCharacter;
class UTextBlock;
class UButton;
class UVerticalBox;

UCLASS()
class FUNGIFIELDS_API UVillagerManagementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Villager Management")
	void SetVillager(AFarmerVillagerCharacter* Villager);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Villager Management")
	void RefreshWidget();

	UFUNCTION()
	void OnHarvesterButtonClicked();

	UFUNCTION()
	void OnPlanterButtonClicked();

	UFUNCTION()
	void OnWatererButtonClicked();

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void HandleRoleChanged(EFarmerRole NewRole);

	UFUNCTION()
	void HandleInventoryChanged();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> VillagerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HarvesterButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlanterButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WatererButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NoToolWarningText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> InventorySummaryBox;

private:
	UPROPERTY()
	TObjectPtr<AFarmerVillagerCharacter> BoundVillager;

	void RefreshToolWarning();
	void RefreshInventorySummary();
	void RefreshRoleHighlight();
};
