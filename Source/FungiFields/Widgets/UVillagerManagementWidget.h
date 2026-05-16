#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../ENUM/EFarmerRole.h"
#include "UVillagerManagementWidget.generated.h"

class AFarmerVillagerCharacter;
class UTextBlock;
class UButton;
class UVerticalBox;
class UBackpackWidget;
class UUniformGridPanel;
class UInventoryComponent;
class UInventorySlotWidget;
struct FInventorySlot;

UCLASS()
class FUNGIFIELDS_API UVillagerManagementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Villager Management")
	void SetVillager(AFarmerVillagerCharacter* Villager);

	/** Hides this widget and starts bed-assignment mode on the player's CropBedSelectionComponent. */
	UFUNCTION(BlueprintCallable, Category = "Villager Management")
	void StartBedAssignment();

	/** Exposes the bound villager to Blueprint (e.g. to access NeedsComponent for hunger bar binding). */
	UFUNCTION(BlueprintPure, Category = "Villager Management")
	AFarmerVillagerCharacter* GetBoundVillager() const { return BoundVillager; }

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

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AssignBedsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NoToolWarningText;

	/**
	 * Dual-inventory drag-drop grids (preferred). Add two UniformGridPanels named
	 * "PlayerInventoryGrid" and "VillagerInventoryGrid" to the Blueprint layout to enable
	 * drag-and-drop item transfer between player and villager inventories.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUniformGridPanel> PlayerInventoryGrid;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUniformGridPanel> VillagerInventoryGrid;

	/** Slot widget Blueprint class — must be set in the Blueprint CDO. */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

	/**
	 * Legacy: WBP_BackPack embedded in the layout. Still works for villager-only display,
	 * but does not support cross-inventory drag-drop. Prefer VillagerInventoryGrid instead.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBackpackWidget> VillagerInventoryWidget;

	/** Legacy text summary box — used only when VillagerInventoryWidget is absent. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> InventorySummaryBox;

	UPROPERTY()
	TObjectPtr<AFarmerVillagerCharacter> BoundVillager;

private:
	static constexpr int32 GridColumns    = 9;
	static constexpr int32 PlayerSlotCount  = 27;
	static constexpr int32 VillagerSlotCount = 18;

	void SetupInventoryGrids();
	void UpdatePlayerSlots();
	void UpdateVillagerSlots();
	UInventorySlotWidget* GetOrCreatePlayerSlotWidget(int32 SlotIndex);
	UInventorySlotWidget* GetOrCreateVillagerSlotWidget(int32 SlotIndex);

	UFUNCTION()
	void OnPlayerInventoryChanged();

	UFUNCTION()
	void HandleSlotDropped(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID);

	bool HandleItemTransfer(int32 SourceSlotIndex, int32 SourceInventoryID, int32 TargetSlotIndex, int32 TargetInventoryID);

	void RefreshToolWarning();
	void RefreshInventorySummary();
	void RefreshRoleHighlight();

	UPROPERTY() TObjectPtr<UInventoryComponent> PlayerInventoryComp;
	UPROPERTY() TObjectPtr<UInventoryComponent> VillagerInventoryComp;
	UPROPERTY() TArray<TObjectPtr<UInventorySlotWidget>> PlayerSlotWidgets;
	UPROPERTY() TArray<TObjectPtr<UInventorySlotWidget>> VillagerSlotWidgets;
};
