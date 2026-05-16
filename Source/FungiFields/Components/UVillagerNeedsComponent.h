#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UVillagerNeedsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHungerChanged, float, NewHungerLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVillagerStarving);

/**
 * Tracks a villager's hunger. Hunger decays over game-time hours.
 * Work speed scales down when hungry and stops entirely when starving.
 * MaxAssignedCropBeds is owned here and scales with villager level.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UVillagerNeedsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVillagerNeedsComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Restore hunger by Amount (clamped to [0, 100]). */
	UFUNCTION(BlueprintCallable, Category = "Needs")
	void Feed(float Amount);

	/** Returns 1.0 (full), 0.5 (hungry), 0.0 (starving). */
	UFUNCTION(BlueprintPure, Category = "Needs")
	float GetWorkSpeedMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Needs")
	float GetHungerLevel() const { return HungerLevel; }

	UFUNCTION(BlueprintPure, Category = "Needs")
	bool IsStarving() const { return HungerLevel <= StarvingThreshold; }

	UFUNCTION(BlueprintPure, Category = "Needs")
	bool IsHungry() const { return HungerLevel <= LowHungerThreshold; }

	UFUNCTION(BlueprintPure, Category = "Needs")
	int32 GetMaxAssignedCropBeds() const { return MaxAssignedCropBeds; }

	/** Call this when the villager levels up to expand their max crop bed assignment. */
	UFUNCTION(BlueprintCallable, Category = "Needs")
	void SetMaxAssignedCropBeds(int32 NewMax) { MaxAssignedCropBeds = FMath::Max(1, NewMax); }

	UPROPERTY(BlueprintAssignable, Category = "Needs")
	FOnHungerChanged OnHungerChanged;

	/** Broadcast once when hunger first crosses below StarvingThreshold. */
	UPROPERTY(BlueprintAssignable, Category = "Needs")
	FOnVillagerStarving OnVillagerStarving;

	// ── Editor config ─────────────────────────────────────────────────────────

	/** Hunger lost per in-game hour (default: full hunger depletes in 20 hours). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float HungerDecayRatePerHour = 5.f;

	/** Below this level the villager works at half speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float LowHungerThreshold = 40.f;

	/** Below this level the villager stops working entirely. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float StarvingThreshold = 10.f;

	/** Starting hunger (0–100). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float StartingHunger = 100.f;

	/** Maximum number of crop beds this villager may be assigned to. Upgradeable with level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "1"))
	int32 MaxAssignedCropBeds = 3;

private:
	UFUNCTION()
	void HandleTimeOfDayChanged(float NewTimeOfDay);

	float HungerLevel = 100.f;
	float PreviousTimeOfDay = -1.f;
	bool bWasStarving = false;
};
