#include "UCropManagerSubsystem.h"
#include "../Components/UCropGrowthComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UCropManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Start the global growth update timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			GrowthUpdateTimerHandle,
			this,
			&UCropManagerSubsystem::OnGrowthUpdateTimer,
			GrowthUpdateInterval,
			true
		);
	}
}

void UCropManagerSubsystem::Deinitialize()
{
	// Clear timer when subsystem is deinitialized
	if (UWorld* World = GetWorld())
	{
		if (GrowthUpdateTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(GrowthUpdateTimerHandle);
		}
	}

	// Clear all registered crops
	RegisteredCrops.Empty();

	Super::Deinitialize();
}

void UCropManagerSubsystem::RegisterCrop(UCropGrowthComponent* GrowthComponent)
{
	if (!GrowthComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCropManagerSubsystem::RegisterCrop: GrowthComponent is null!"));
		return;
	}

	RegisteredCrops.Add(GrowthComponent);
}

void UCropManagerSubsystem::UnregisterCrop(UCropGrowthComponent* GrowthComponent)
{
	if (!GrowthComponent)
	{
		return;
	}

	RegisteredCrops.Remove(GrowthComponent);
}

void UCropManagerSubsystem::PauseAllGrowth()
{
	bGrowthPaused = true;
}

void UCropManagerSubsystem::ResumeAllGrowth()
{
	bGrowthPaused = false;
}

void UCropManagerSubsystem::OnGrowthUpdateTimer()
{
	if (bGrowthPaused)
	{
		return;
	}

	// Update all registered crops
	// Use a copy of the set to avoid issues if crops unregister during iteration
	TArray<TObjectPtr<UCropGrowthComponent>> CropsToUpdate(RegisteredCrops.Array());

	for (UCropGrowthComponent* GrowthComponent : CropsToUpdate)
	{
		if (IsValid(GrowthComponent))
		{
			// Call the growth update method on each component
			GrowthComponent->UpdateGrowth(GrowthUpdateInterval);
		}
		else
		{
			// Remove invalid components
			RegisteredCrops.Remove(GrowthComponent);
		}
	}
}
