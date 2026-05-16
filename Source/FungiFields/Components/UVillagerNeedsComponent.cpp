#include "UVillagerNeedsComponent.h"
#include "../Subsystems/UDayNightSubsystem.h"

UVillagerNeedsComponent::UVillagerNeedsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVillagerNeedsComponent::BeginPlay()
{
	Super::BeginPlay();

	HungerLevel = StartingHunger;
	bWasStarving = false;

	if (UWorld* World = GetWorld())
	{
		if (UDayNightSubsystem* DayNight = World->GetSubsystem<UDayNightSubsystem>())
		{
			PreviousTimeOfDay = DayNight->GetTimeOfDay();
			DayNight->OnTimeOfDayChanged.AddDynamic(this, &UVillagerNeedsComponent::HandleTimeOfDayChanged);
		}
	}
}

void UVillagerNeedsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UDayNightSubsystem* DayNight = World->GetSubsystem<UDayNightSubsystem>())
			DayNight->OnTimeOfDayChanged.RemoveDynamic(this, &UVillagerNeedsComponent::HandleTimeOfDayChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void UVillagerNeedsComponent::HandleTimeOfDayChanged(float NewTimeOfDay)
{
	if (PreviousTimeOfDay < 0.f)
	{
		PreviousTimeOfDay = NewTimeOfDay;
		return;
	}

	// Handle midnight wraparound (e.g. 23.9 -> 0.1)
	float DeltaHours = NewTimeOfDay - PreviousTimeOfDay;
	if (DeltaHours < 0.f)
		DeltaHours += 24.f;

	PreviousTimeOfDay = NewTimeOfDay;

	if (DeltaHours <= 0.f || HungerDecayRatePerHour <= 0.f)
		return;

	const float OldHunger = HungerLevel;
	HungerLevel = FMath::Max(0.f, HungerLevel - HungerDecayRatePerHour * DeltaHours);

	if (FMath::Abs(HungerLevel - OldHunger) > SMALL_NUMBER)
		OnHungerChanged.Broadcast(HungerLevel);

	if (!bWasStarving && HungerLevel <= StarvingThreshold)
	{
		bWasStarving = true;
		OnVillagerStarving.Broadcast();
	}
	else if (bWasStarving && HungerLevel > StarvingThreshold)
	{
		bWasStarving = false;
	}
}

void UVillagerNeedsComponent::Feed(float Amount)
{
	if (Amount <= 0.f) return;

	const float OldHunger = HungerLevel;
	HungerLevel = FMath::Min(100.f, HungerLevel + Amount);

	if (FMath::Abs(HungerLevel - OldHunger) > SMALL_NUMBER)
	{
		OnHungerChanged.Broadcast(HungerLevel);

		if (bWasStarving && HungerLevel > StarvingThreshold)
			bWasStarving = false;
	}
}

float UVillagerNeedsComponent::GetWorkSpeedMultiplier() const
{
	if (HungerLevel <= StarvingThreshold) return 0.f;
	if (HungerLevel <= LowHungerThreshold) return 0.5f;
	return 1.f;
}
