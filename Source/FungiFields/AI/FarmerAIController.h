#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "../ENUM/EToolType.h"
#include "../ENUM/EFarmerJobState.h"
#include "FarmerAIController.generated.h"

class UFarmingComponent;
class UFarmerTargetingComponent;
class AFarmerVillagerCharacter;
class UToolDataAsset;
class USeedDataAsset;

/**
 * Timer-driven controller for autonomous farming tasks.
 * Supports Harvester, Planter, and Waterer roles.
 * Requires physical tools and seeds in the villager's UInventoryComponent to operate.
 */
UCLASS()
class FUNGIFIELDS_API AFarmerAIController : public AAIController
{
	GENERATED_BODY()

public:
	AFarmerAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	/** Called by AFarmerVillagerCharacter::SetAssignedRole to restart the brain loop cleanly. */
	void ResetToIdle();

protected:
	void TickBrain();
	void AcquireTargetForRole();
	void AcquireHarvestTarget();
	void AcquirePlantTarget();
	void AcquireWaterTarget();
	void ProcessCurrentTarget();
	void BeginWander();
	void BeginCooldown();
	void HandleCooldownFinished();
	void ClearCurrentTarget();
	bool TryGetTargetActionData(FVector& OutActionLocation, float& OutInteractionRange) const;
	bool IsTargetInRange(const FVector& ActionLocation, float InteractionRange) const;
	EToolType GetActiveToolType() const;
	float GetActiveToolPower() const;
	bool IsHarvesterRoleActive() const;
	bool IsDebugEnabled() const;
	void LogDebug(const FString& Message, ELogVerbosity::Type Verbosity = ELogVerbosity::Log) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Farmer", meta = (ClampMin = "0.05"))
	float BrainTickInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Farmer", meta = (ClampMin = "0.05"))
	float ActionCooldownSeconds = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Farmer", meta = (ClampMin = "0.0"))
	float MoveAcceptanceBuffer = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Farmer", meta = (ClampMin = "1.0"))
	float FallbackInteractionRange = 100.0f;

private:
	UPROPERTY()
	TObjectPtr<UFarmingComponent> FarmingComponent;

	UPROPERTY()
	TObjectPtr<UFarmerTargetingComponent> TargetingComponent;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	UPROPERTY()
	TObjectPtr<AFarmerVillagerCharacter> CachedVillager;

	UPROPERTY()
	TObjectPtr<UToolDataAsset> CachedToolData;

	UPROPERTY()
	TObjectPtr<USeedDataAsset> CachedSeedData;

	int32 CachedSeedSlotIndex = INDEX_NONE;

	FTimerHandle BrainTimerHandle;
	FTimerHandle CooldownTimerHandle;
	FTimerHandle WanderCooldownTimer;

	EFarmerJobState JobState = EFarmerJobState::Idle;
};
