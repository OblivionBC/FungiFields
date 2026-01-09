#include "UCropManagerSubsystem.h"
#include "../Components/UCropGrowthComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UCropManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

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
	if (UWorld* World = GetWorld())
	{
		if (GrowthUpdateTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(GrowthUpdateTimerHandle);
		}
	}

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

	TArray<TObjectPtr<UCropGrowthComponent>> CropsToUpdate(RegisteredCrops.Array());

	for (UCropGrowthComponent* GrowthComponent : CropsToUpdate)
	{
		if (IsValid(GrowthComponent))
		{
			GrowthComponent->UpdateGrowth(GrowthUpdateInterval);
		}
		else
		{
			RegisteredCrops.Remove(GrowthComponent);
		}
	}
}



