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

class AActor;

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

	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UseEquippedTool(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Farming")
	void SetEquippedTool(EToolType ToolType, float ToolPower);

	UFUNCTION(BlueprintPure, Category = "Farming")
	EToolType GetCurrentToolType() const { return CurrentToolType; }

	UFUNCTION(BlueprintPure, Category = "Farming")
	float GetCurrentToolPower() const { return CurrentToolPower; }

	UFUNCTION(BlueprintPure, Category = "Farming")
	bool HasValidToolEquipped() const { return bHasValidTool; }

	/** Should be called from owner's BeginPlay after components are initialized. */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void SetCamera(UCameraComponent* Camera);

	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UpdateEquippedTool();

	/** Works for both players (camera-based) and NPCs (location-based). */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool ExecuteFarmingAction(AActor* TargetActor, const FVector& ActionLocation, EToolType ToolType, float ToolPower, USeedDataAsset* SeedData = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool CanPerformFarmingAction(AActor* TargetActor, EToolType ToolType, USeedDataAsset* SeedData = nullptr) const;

	UFUNCTION(BlueprintPure, Category = "Farming")
	bool HasSeedEquipped() const { return bHasSeedEquipped; }

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
	bool PerformToolTrace(FHitResult& OutHit) const;
	void TraceForFarmable();
	void ShowFarmingTooltip(AActor* Target, const FText& Prompt);
	void HideFarmingTooltip();
	void ClearFarmable();

	/** @return True if stamina was available and consumed */
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

	/** The last tooltip text that was displayed */
	FText LastTooltipText;

	/** Timer handle for clearing the widget after losing focus */
	FTimerHandle FarmableResetTimer;

	/** Instance of the farming tooltip widget */
	UPROPERTY()
	UUserWidget* FarmingTooltipWidget = nullptr;
};
