#include "ABedActor.h"
#include "Components/StaticMeshComponent.h"
#include "../Subsystems/UDayNightSubsystem.h"
#include "Engine/World.h"

ABedActor::ABedActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

void ABedActor::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UDayNightSubsystem* DayNight = World->GetSubsystem<UDayNightSubsystem>())
		{
			DayNight->SkipToMorning();
		}
	}
}

FText ABedActor::GetInteractionText_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (UDayNightSubsystem* DayNight = World->GetSubsystem<UDayNightSubsystem>())
		{
			if (DayNight->IsNight())
			{
				return FText::FromString(TEXT("Sleep until morning"));
			}
		}
	}
	return FText::FromString(TEXT("Rest until morning"));
}

FText ABedActor::GetTooltipText_Implementation() const
{
	return BedName;
}
