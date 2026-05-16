#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UCropBedSelectionComponent.generated.h"

class AFarmerVillagerCharacter;
class ASoilPlot;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionModeChanged, bool, bIsActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoveredPlotChanged,   ASoilPlot*, HoveredPlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBedCountChanged, int32, CurrentBeds, int32, MaxBeds);

/**
 * Attaches to the player character. When activated via StartSelection(), the normal interact
 * input (E) assigns/unassigns soil plots to the target villager. Escape exits the mode.
 *
 * The selection overlay widget is a pure HUD — no cursor/buttons. It should display:
 *   - Villager name
 *   - "Beds: X / Y" (driven by OnBedCountChanged)
 *   - Static hint text: "Aim at a soil plot and press [E] • Escape to finish"
 *
 * Blueprint highlight logic hooks into OnHoveredPlotChanged.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UCropBedSelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCropBedSelectionComponent();

	/** Enter bed-assignment mode for the given villager. Shows overlay HUD. */
	UFUNCTION(BlueprintCallable, Category = "Bed Selection")
	void StartSelection(AFarmerVillagerCharacter* Villager);

	/** Exit bed-assignment mode and remove the overlay. */
	UFUNCTION(BlueprintCallable, Category = "Bed Selection")
	void EndSelection();

	/** Called by InteractionComponent instead of normal interaction while active. */
	void HandleInteract();

	UFUNCTION(BlueprintPure, Category = "Bed Selection")
	bool IsSelectionActive() const { return TargetVillager.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Bed Selection")
	AFarmerVillagerCharacter* GetTargetVillager() const { return TargetVillager.Get(); }

	/** Plot currently under the crosshair, or nullptr. Used for Blueprint highlight effects. */
	UFUNCTION(BlueprintPure, Category = "Bed Selection")
	ASoilPlot* GetHoveredPlot() const { return HoveredPlot.Get(); }

	/** How many beds the target villager currently has assigned. */
	UFUNCTION(BlueprintPure, Category = "Bed Selection")
	int32 GetCurrentAssignedBeds() const;

	/** MaxAssignedCropBeds from the villager's NeedsComponent. */
	UFUNCTION(BlueprintPure, Category = "Bed Selection")
	int32 GetMaxBeds() const;

	/** Display name of the target villager, for the overlay header. */
	UFUNCTION(BlueprintPure, Category = "Bed Selection")
	FText GetTargetVillagerName() const;

	/** Fires when active and the hovered plot changes (including to nullptr). */
	UPROPERTY(BlueprintAssignable, Category = "Bed Selection")
	FOnHoveredPlotChanged OnHoveredPlotChanged;

	/** Fires when entering/leaving selection mode. */
	UPROPERTY(BlueprintAssignable, Category = "Bed Selection")
	FOnSelectionModeChanged OnSelectionModeChanged;

	/** Fires when the assigned-bed count changes. Bind the overlay's "X / Y" text to this. */
	UPROPERTY(BlueprintAssignable, Category = "Bed Selection")
	FOnBedCountChanged OnBedCountChanged;

	/** Widget class for the HUD overlay (assign in CDO). Must be a pure informational widget — no buttons. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bed Selection")
	TSubclassOf<UUserWidget> SelectionOverlayWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bed Selection", meta = (ClampMin = "100.0"))
	float TraceDistance = 800.f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	ASoilPlot* TraceForPlot() const;
	UCameraComponent* GetCamera() const;

	/** Re-applies highlight state to all currently assigned plots + hovered plot. */
	void RefreshAllHighlights();

	/** Clears highlight on all assigned + hovered plots before leaving selection mode. */
	void ClearAllHighlights();

	UFUNCTION()
	void HandleVillagerPlotsChanged();

	UPROPERTY() TWeakObjectPtr<AFarmerVillagerCharacter> TargetVillager;
	UPROPERTY() TWeakObjectPtr<ASoilPlot>               HoveredPlot;
	UPROPERTY() TObjectPtr<UUserWidget>                  OverlayWidget;
};
