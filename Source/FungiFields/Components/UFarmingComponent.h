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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFarmingActionPerformed, AActor*, Instigator);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UFarmingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFarmingComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnCropHarvested OnCropHarvested;

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnSeedPlanted OnSeedPlanted;

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnSoilTilled OnSoilTilled;

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnSoilWatered OnSoilWatered;

	UPROPERTY(BlueprintAssignable, Category = "Farming Events")
	FOnFarmingActionPerformed OnFarmingActionPerformed;

protected:
	bool PerformToolTrace(FHitResult& OutHit) const;
	void TraceForFarmable();
	void ShowFarmingTooltip(AActor* Target, const FText& Prompt);
	void HideFarmingTooltip();
	void ClearFarmable();

	bool ConsumeStamina(float StaminaCost);
	float ResolveStaminaCost() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farming Settings")
	TSubclassOf<UUserWidget> FarmingTooltipWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farming Settings", meta = (ClampMin = "0.0"))
	float TooltipTraceDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farming Settings", meta = (ClampMin = "0.0"))
	float TooltipClearDelay = 3.0f;

private:
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	EToolType CurrentToolType = EToolType::Hoe;

	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	float CurrentToolPower = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	bool bHasValidTool = false;

	UPROPERTY(EditAnywhere, Category = "Farming Settings", meta = (ClampMin = "0.0"))
	float ToolTraceDistance = 800.0f;

	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	bool bHasSeedEquipped = false;

	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	TObjectPtr<USeedDataAsset> EquippedSeedData = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	int32 EquippedSlotIndexCached = INDEX_NONE;

private:
	UPROPERTY()
	AActor* LastFarmableTarget = nullptr;

	FText LastTooltipText;

	FTimerHandle FarmableResetTimer;
	FTimerHandle TooltipPollingTimer;

	UPROPERTY()
	UUserWidget* FarmingTooltipWidget = nullptr;
};
