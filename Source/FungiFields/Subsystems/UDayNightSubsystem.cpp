#include "UDayNightSubsystem.h"
#include "UPricingSubsystem.h"
#include "UCropManagerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UDayNightSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TimeOfDay = StartingHour;
	bIsCurrentlyNight = (TimeOfDay >= NightStartHour || TimeOfDay < DayStartHour);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ClockTimerHandle,
			this,
			&UDayNightSubsystem::OnClockTick,
			ClockTickInterval,
			true
		);
	}
}

void UDayNightSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ClockTimerHandle);
	}
	Super::Deinitialize();
}

bool UDayNightSubsystem::IsNight() const
{
	return bIsCurrentlyNight;
}

void UDayNightSubsystem::SkipToMorning()
{
	const float OldTime = TimeOfDay;
	TimeOfDay = DayStartHour;

	AdvanceDay();

	if (bIsCurrentlyNight)
	{
		bIsCurrentlyNight = false;
		BroadcastDayStarted();
	}

	OnTimeOfDayChanged.Broadcast(TimeOfDay);
}

void UDayNightSubsystem::SetTimeOfDay(float NewTime)
{
	const float OldTime = TimeOfDay;
	TimeOfDay = FMath::Fmod(FMath::Max(0.0f, NewTime), 24.0f);
	HandleDayNightTransitions(OldTime, TimeOfDay);
	OnTimeOfDayChanged.Broadcast(TimeOfDay);
}

void UDayNightSubsystem::OnClockTick()
{
	const float OldTime = TimeOfDay;
	const float HoursPerTick = ClockTickInterval / SecondsPerGameHour;
	TimeOfDay += HoursPerTick;

	if (TimeOfDay >= 24.0f)
	{
		TimeOfDay -= 24.0f;
		AdvanceDay();
	}

	HandleDayNightTransitions(OldTime, TimeOfDay);
	OnTimeOfDayChanged.Broadcast(TimeOfDay);
}

void UDayNightSubsystem::HandleDayNightTransitions(float OldTime, float NewTime)
{
	const bool bWasNight = bIsCurrentlyNight;
	bIsCurrentlyNight = (NewTime >= NightStartHour || NewTime < DayStartHour);

	if (!bWasNight && bIsCurrentlyNight)
	{
		BroadcastNightStarted();
	}
	else if (bWasNight && !bIsCurrentlyNight)
	{
		BroadcastDayStarted();
	}
}

void UDayNightSubsystem::BroadcastDayStarted()
{
	if (UWorld* World = GetWorld())
	{
		if (UCropManagerSubsystem* CropManager = World->GetSubsystem<UCropManagerSubsystem>())
		{
			CropManager->ResumeNonNightCrops();
		}
	}

	OnDayStarted.Broadcast();
}

void UDayNightSubsystem::BroadcastNightStarted()
{
	if (UWorld* World = GetWorld())
	{
		if (UCropManagerSubsystem* CropManager = World->GetSubsystem<UCropManagerSubsystem>())
		{
			CropManager->PauseNonNightCrops();
		}
	}

	OnNightStarted.Broadcast();
}

void UDayNightSubsystem::AdvanceDay()
{
	CurrentDay++;

	if (UWorld* World = GetWorld())
	{
		if (UPricingSubsystem* Pricing = World->GetSubsystem<UPricingSubsystem>())
		{
			Pricing->OnDayAdvanced();
		}
	}

	OnNewDayAdvanced.Broadcast();
}
