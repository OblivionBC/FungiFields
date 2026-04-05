#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "../ENUM/EToolType.h"
#include "../ENUM/EFarmerJobState.h"
#include "FarmerAIController.generated.h"

class UFarmingComponent;
class UFarmerTargetingComponent;
class AFarmerVillagerCharacter;

/**
 * Timer-driven controller for autonomous farming tasks.
 * Implements the first vertical slice role: Harvester.
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

protected:
	void TickBrain();
	void AcquireHarvestTarget();
	void AcquireTargetForRole();
	void ProcessCurrentTarget();
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

	/** Main AI polling interval for this role loop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Harvester", meta = (ClampMin = "0.05"))
	float BrainTickInterval = 0.5f;

	/** Cooldown after each harvest attempt to prevent spam and allow retargeting. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Harvester", meta = (ClampMin = "0.05"))
	float ActionCooldownSeconds = 0.4f;

	/** Extra move acceptance distance added to the target interaction range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Harvester", meta = (ClampMin = "0.0"))
	float MoveAcceptanceBuffer = 15.0f;

	/** Fallback interaction range if target does not provide one. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Harvester", meta = (ClampMin = "1.0"))
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

	FTimerHandle BrainTimerHandle;
	FTimerHandle CooldownTimerHandle;

	EFarmerJobState JobState = EFarmerJobState::Idle;
};
