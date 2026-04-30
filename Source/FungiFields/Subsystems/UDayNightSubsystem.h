#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UDayNightSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDayStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNightStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNewDayAdvanced);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, float, TimeOfDay);

/**
 * World subsystem that drives an in-game clock and broadcasts day/night transitions.
 * Uses a timer (not Tick) to advance TimeOfDay on a configurable real-time interval.
 *
 * Automatically wires to UPricingSubsystem::OnDayAdvanced and UCropManagerSubsystem
 * pause/resume on day/night transitions.
 */
UCLASS()
class FUNGIFIELDS_API UDayNightSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Current in-game hour (0.0 .. 24.0). */
	UFUNCTION(BlueprintPure, Category = "Day Night")
	float GetTimeOfDay() const { return TimeOfDay; }

	UFUNCTION(BlueprintPure, Category = "Day Night")
	bool IsNight() const;

	UFUNCTION(BlueprintPure, Category = "Day Night")
	int32 GetCurrentDay() const { return CurrentDay; }

	/**
	 * Fast-forwards the clock to the next DayStartHour.
	 * Fires OnNewDayAdvanced and OnDayStarted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Day Night")
	void SkipToMorning();

	/** Manually set the time (useful for debug / Blueprint scripting). */
	UFUNCTION(BlueprintCallable, Category = "Day Night")
	void SetTimeOfDay(float NewTime);

	UPROPERTY(BlueprintAssignable, Category = "Day Night Events")
	FOnDayStarted OnDayStarted;

	UPROPERTY(BlueprintAssignable, Category = "Day Night Events")
	FOnNightStarted OnNightStarted;

	/** Fires once per in-game day transition (midnight or SkipToMorning). */
	UPROPERTY(BlueprintAssignable, Category = "Day Night Events")
	FOnNewDayAdvanced OnNewDayAdvanced;

	UPROPERTY(BlueprintAssignable, Category = "Day Night Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	/** Hour at which daytime begins (default 6). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Day Night Settings", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float DayStartHour = 6.0f;

	/** Hour at which nighttime begins (default 20). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Day Night Settings", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float NightStartHour = 20.0f;

	/** Real-world seconds per in-game hour. Lower = faster days. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Day Night Settings", meta = (ClampMin = "0.1"))
	float SecondsPerGameHour = 60.0f;

	/** Hour the clock starts at when the world loads. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Day Night Settings", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float StartingHour = 8.0f;

private:
	UFUNCTION()
	void OnClockTick();

	void HandleDayNightTransitions(float OldTime, float NewTime);
	void BroadcastDayStarted();
	void BroadcastNightStarted();
	void AdvanceDay();

	float TimeOfDay = 8.0f;
	int32 CurrentDay = 1;
	bool bIsCurrentlyNight = false;

	FTimerHandle ClockTimerHandle;

	static constexpr float ClockTickInterval = 0.5f;
};
