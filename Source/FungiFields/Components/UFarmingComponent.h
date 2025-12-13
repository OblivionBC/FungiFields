#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../ENUM/EToolType.h"
#include "../Data/UCropDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "UFarmingComponent.generated.h"

class UCameraComponent;
class UInventoryComponent;
class UCharacterAttributeSet;
class UToolDataAsset;
class UItemDataAsset;
struct FInputActionValue;

// Forward declarations
class AActor;

// Delegate declarations for farming actions
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCropHarvested, AActor*, Harvester, UCropDataAsset*, CropData, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSeedPlanted, AActor*, Planter, USeedDataAsset*, SeedData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoilTilled, AActor*, Tiller);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSoilWatered, AActor*, Waterer, AActor*, SoilPlot);

/**
 * Component responsible for handling farming tool usage.
 * Performs line traces and interacts with farmable/harvestable actors via interfaces.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UFarmingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFarmingComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	/**
	 * Use the currently equipped tool.
	 * Performs line trace and interacts with target actor.
	 * @param Value Input action value (unused, but required for input binding)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UseEquippedTool(const FInputActionValue& Value);

	/**
	 * Set the equipped tool from inventory.
	 * Called when inventory changes or tool is equipped.
	 * @param ToolType Type of tool
	 * @param ToolPower Power/effectiveness of tool
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void SetEquippedTool(EToolType ToolType, float ToolPower);

	/**
	 * Get the currently equipped tool type.
	 * @return Tool type, or EToolType::Hoe if no tool equipped
	 */
	UFUNCTION(BlueprintPure, Category = "Farming")
	EToolType GetCurrentToolType() const { return CurrentToolType; }

	/**
	 * Get the currently equipped tool power.
	 * @return Tool power
	 */
	UFUNCTION(BlueprintPure, Category = "Farming")
	float GetCurrentToolPower() const { return CurrentToolPower; }

	/**
	 * Check if a valid tool is currently equipped.
	 * @return True if a tool is equipped, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Farming")
	bool HasValidToolEquipped() const { return bHasValidTool; }

	/**
	 * Set the camera component to use for tool traces.
	 * Should be called from owner's BeginPlay after components are initialized.
	 * @param Camera The camera component to use for line traces
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void SetCamera(UCameraComponent* Camera);

	/**
	 * Update equipped tool from inventory.
	 * Checks equipped slot and extracts tool data if it's a tool.
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UpdateEquippedTool();

	/**
	 * Execute a farming action on a target actor at a specific location.
	 * Works for both players (camera-based) and NPCs (location-based).
	 * @param TargetActor The actor to perform the action on
	 * @param ActionLocation The world location where the action is being performed
	 * @param ToolType The type of tool being used (or None for planting)
	 * @param ToolPower The power of the tool
	 * @param SeedData Optional seed data if planting
	 * @return True if the action was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool ExecuteFarmingAction(AActor* TargetActor, const FVector& ActionLocation, EToolType ToolType, float ToolPower, USeedDataAsset* SeedData = nullptr);

	/**
	 * Check if a farming action can be performed on a target actor.
	 * @param TargetActor The actor to check
	 * @param ToolType The type of tool (or None for planting)
	 * @param SeedData Optional seed data if checking planting
	 * @return True if the action can be performed
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool CanPerformFarmingAction(AActor* TargetActor, EToolType ToolType, USeedDataAsset* SeedData = nullptr) const;

	/**
	 * Check if a seed is currently equipped.
	 * @return True if a seed is equipped
	 */
	UFUNCTION(BlueprintPure, Category = "Farming")
	bool HasSeedEquipped() const { return bHasSeedEquipped; }

	/**
	 * Get the currently equipped seed data.
	 * @return Seed data asset, or nullptr if no seed equipped
	 */
	UFUNCTION(BlueprintPure, Category = "Farming")
	USeedDataAsset* GetEquippedSeedData() const { return EquippedSeedData; }

	/** Delegate broadcast when a crop is harvested */
	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnCropHarvested OnCropHarvested;

	/** Delegate broadcast when a seed is planted */
	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnSeedPlanted OnSeedPlanted;

	/** Delegate broadcast when soil is tilled */
	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnSoilTilled OnSoilTilled;

	/** Delegate broadcast when soil is watered */
	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnSoilWatered OnSoilWatered;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:

	/**
	 * Perform line trace for tool interaction.
	 * @param OutHit Hit result if trace succeeds
	 * @return True if trace hit something
	 */
	bool PerformToolTrace(FHitResult& OutHit) const;

	/**
	 * Performs a line trace from the camera to detect farmable/harvestable actors.
	 * Called every frame in TickComponent.
	 */
	void TraceForFarmable();

	/**
	 * Shows the farming tooltip widget with the specified prompt text.
	 * @param Target The actor that can be farmed/harvested
	 * @param Prompt The text to display in the tooltip widget
	 */
	void ShowFarmingTooltip(AActor* Target, const FText& Prompt);

	/**
	 * Hides the farming tooltip widget.
	 */
	void HideFarmingTooltip();

	/**
	 * Clears the current farmable reference and hides the widget.
	 * Called by timer when no farmable is detected.
	 */
	void ClearFarmable();

	/**
	 * Consume stamina for tool usage.
	 * @param StaminaCost Amount of stamina to consume
	 * @return True if stamina was available and consumed
	 */
	bool ConsumeStamina(float StaminaCost);

	/** Widget class to use for displaying farming tooltips */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farming Settings")
	TSubclassOf<UUserWidget> FarmingTooltipWidgetClass;

	/** Maximum distance for farming tooltip traces */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farming Settings", meta = (ClampMin = "0.0"))
	float TooltipTraceDistance = 800.0f;

	/** Delay in seconds before clearing the widget when no farmable is detected */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farming Settings", meta = (ClampMin = "0.0"))
	float TooltipClearDelay = 3.0f;

private:
	/** Camera component for line traces */
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;

	/** Currently equipped tool type */
	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	EToolType CurrentToolType = EToolType::Hoe;

	/** Currently equipped tool power */
	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	float CurrentToolPower = 1.0f;

	/** Whether a valid tool is currently equipped */
	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	bool bHasValidTool = false;

	/** Maximum distance for tool interaction */
	UPROPERTY(EditAnywhere, Category = "Farming Settings", meta = (ClampMin = "0.0"))
	float ToolTraceDistance = 800.0f;

	/** Whether a seed is currently equipped */
	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	bool bHasSeedEquipped = false;

	/** Data asset for the currently equipped seed */
	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	TObjectPtr<USeedDataAsset> EquippedSeedData = nullptr;

	/** Cached index of the equipped slot in inventory */
	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	int32 EquippedSlotIndexCached = INDEX_NONE;

private:
	/** The currently focused farmable/harvestable actor */
	UPROPERTY()
	AActor* LastFarmableTarget = nullptr;

	/** Timer handle for clearing the widget after losing focus */
	FTimerHandle FarmableResetTimer;

	/** Instance of the farming tooltip widget */
	UPROPERTY()
	UUserWidget* FarmingTooltipWidget = nullptr;
};
