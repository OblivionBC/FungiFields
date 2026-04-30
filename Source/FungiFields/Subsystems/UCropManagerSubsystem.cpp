#include "UCropManagerSubsystem.h"
#include "../Components/UCropGrowthComponent.h"
#include "../Data/UCropDataAsset.h"
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

	if (!RegisteredCrops.Contains(GrowthComponent))
		RegisteredCrops.Add(GrowthComponent);

	GrowthComponent->OnCropFullyGrown.AddDynamic(this, &UCropManagerSubsystem::HandleCropFullyGrown);
	GrowthComponent->OnCropWithered.AddDynamic(this, &UCropManagerSubsystem::HandleCropWithered);
}

void UCropManagerSubsystem::UnregisterCrop(UCropGrowthComponent* GrowthComponent)
{
	if (!GrowthComponent)
	{
		return;
	}

	RegisteredCrops.Remove(GrowthComponent);

	GrowthComponent->OnCropFullyGrown.RemoveDynamic(this, &UCropManagerSubsystem::HandleCropFullyGrown);
	GrowthComponent->OnCropWithered.RemoveDynamic(this, &UCropManagerSubsystem::HandleCropWithered);
}

void UCropManagerSubsystem::PauseAllGrowth()
{
	if (UWorld* World = GetWorld())
		World->GetTimerManager().PauseTimer(GrowthUpdateTimerHandle);
}

void UCropManagerSubsystem::ResumeAllGrowth()
{
	if (UWorld* World = GetWorld())
		World->GetTimerManager().UnPauseTimer(GrowthUpdateTimerHandle);
}

void UCropManagerSubsystem::PauseNonNightCrops()
{
	NightPausedCrops.Empty();

	for (UCropGrowthComponent* GrowthComponent : RegisteredCrops)
	{
		if (!IsValid(GrowthComponent))
		{
			continue;
		}

		UCropDataAsset* CropData = GrowthComponent->GetCropData();
		if (!CropData || !CropData->bGrowsAtNight)
		{
			NightPausedCrops.Add(GrowthComponent);
		}
	}
}

void UCropManagerSubsystem::ResumeNonNightCrops()
{
	NightPausedCrops.Empty();
}

void UCropManagerSubsystem::OnGrowthUpdateTimer()
{
	TArray<TObjectPtr<UCropGrowthComponent>> CropsToUpdate(RegisteredCrops.Array());

	for (UCropGrowthComponent* GrowthComponent : CropsToUpdate)
	{
		if (!IsValid(GrowthComponent))
		{
			RegisteredCrops.Remove(GrowthComponent);
			NightPausedCrops.Remove(GrowthComponent);
			continue;
		}

		if (NightPausedCrops.Contains(GrowthComponent))
		{
			continue;
		}

		GrowthComponent->UpdateGrowth(GrowthUpdateInterval);
	}
}

void UCropManagerSubsystem::HandleCropFullyGrown(AActor* Crop)
{
	OnCropFullyGrown.Broadcast(Crop);
}

void UCropManagerSubsystem::HandleCropWithered(AActor* Crop)
{
	OnCropWithered.Broadcast(Crop);
}




