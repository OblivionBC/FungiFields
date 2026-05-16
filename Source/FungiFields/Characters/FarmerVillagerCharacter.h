#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "../ENUM/EToolType.h"
#include "../ENUM/EFarmerRole.h"
#include "../Interfaces/IInteractableInterface.h"
#include "FarmerVillagerCharacter.generated.h"

class UFarmingComponent;
class UInventoryComponent;
class UFarmerTargetingComponent;
class UVillagerNeedsComponent;
class ASoilPlot;
class UToolDataAsset;
class USeedDataAsset;
class UCropDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVillagerRoleChanged, EFarmerRole, NewRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAssignedPlotsChanged);

/**
 * Autonomous villager character owned by the player's farm.
 * Owns reusable farming components and exposes role configuration to the AI controller.
 * Implements IInteractableInterface so the player can assign roles via the management UI.
 */
UCLASS()
class FUNGIFIELDS_API AFarmerVillagerCharacter : public ACharacter, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AFarmerVillagerCharacter();

	virtual void BeginPlay() override;

	// IInteractableInterface
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;
	virtual FText GetTooltipText_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	UFarmingComponent* GetFarmingComponent() const { return FarmingComponent; }

	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	UFarmerTargetingComponent* GetTargetingComponent() const { return TargetingComponent; }

	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "AI|Needs")
	UVillagerNeedsComponent* GetNeedsComponent() const { return NeedsComponent; }

	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	EFarmerRole GetAssignedRole() const { return AssignedRole; }

	UFUNCTION(BlueprintPure, Category = "Villager")
	FText GetVillagerDisplayName() const { return VillagerDisplayName; }

	UFUNCTION(BlueprintCallable, Category = "AI|Role")
	void SetAssignedRole(EFarmerRole NewRole);

	/**
	 * Returns the first inventory slot containing a tool matching the required type for the current role.
	 * Returns nullptr if no valid tool is found.
	 */
	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	UToolDataAsset* FindEquippedToolForRole() const;

	/**
	 * Returns the first inventory slot containing any USeedDataAsset.
	 * OutSlotIndex is set to the found slot index, or INDEX_NONE if no seed found.
	 */
	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	USeedDataAsset* FindSeedInInventory(int32& OutSlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "AI|Debug")
	bool IsAIDebugLoggingEnabled() const { return bEnableAIDebugLogs; }

	UFUNCTION(BlueprintPure, Category = "AI|Wander")
	FVector GetHomeLocation() const { return HomeLocation; }

	UPROPERTY(BlueprintAssignable, Category = "AI|Role")
	FOnVillagerRoleChanged OnRoleChanged;

	UPROPERTY(BlueprintAssignable, Category = "AI|Beds")
	FOnAssignedPlotsChanged OnAssignedPlotsChanged;

	/** Assign a soil plot to this villager. Returns false if already at MaxAssignedCropBeds or already assigned. */
	UFUNCTION(BlueprintCallable, Category = "AI|Beds")
	bool AssignPlot(ASoilPlot* Plot);

	UFUNCTION(BlueprintCallable, Category = "AI|Beds")
	void UnassignPlot(ASoilPlot* Plot);

	UFUNCTION(BlueprintPure, Category = "AI|Beds")
	bool IsPlotAssigned(const ASoilPlot* Plot) const;

	UFUNCTION(BlueprintPure, Category = "AI|Beds")
	int32 GetAssignedPlotCount() const;

	/** Returns assigned plots as a flat array (removes any stale entries first). */
	TArray<TWeakObjectPtr<ASoilPlot>>& GetAssignedPlotsRaw() { return AssignedPlots; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Wander", meta = (ClampMin = "0.0"))
	float WanderRadius = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Wander", meta = (ClampMin = "0.05"))
	float WanderCooldownMin = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Wander", meta = (ClampMin = "0.05"))
	float WanderCooldownMax = 8.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFarmingComponent> FarmingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFarmerTargetingComponent> TargetingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UVillagerNeedsComponent> NeedsComponent;

	/** Display name shown in UI and interaction prompts. Set per-instance in the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Villager")
	FText VillagerDisplayName = FText::FromString(TEXT("Villager"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Role")
	EFarmerRole AssignedRole = EFarmerRole::Harvester;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Debug")
	bool bEnableAIDebugLogs = true;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> VillagerManagementWidgetClass;

private:
	void OpenManagementWidget(APlayerController* PC);

	/**
	 * If the interactor has a food item equipped, feed this villager and consume one item.
	 * Returns true if feeding occurred (caller should then skip the dialogue path).
	 */
	bool TryFeedFromInteractor(AActor* Interactor);

	UFUNCTION()
	void OnDialogueManageRequested();

	FVector HomeLocation = FVector::ZeroVector;

	/** Cached controller from the last interaction, used by dialogue callback. */
	TWeakObjectPtr<APlayerController> LastInteractorPC;

	TArray<TWeakObjectPtr<ASoilPlot>> AssignedPlots;

	EToolType ResolveRoleToolType(EFarmerRole FarmerRole) const;

	UFUNCTION()
	void OnCropHarvestedForMilestone(AActor* Harvester, UCropDataAsset* CropData, int32 Quantity);

	UFUNCTION()
	void OnSeedPlantedForMilestone(AActor* Planter, USeedDataAsset* SeedData);
};
