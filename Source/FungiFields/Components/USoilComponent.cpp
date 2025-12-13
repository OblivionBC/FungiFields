#include "USoilComponent.h"
#include "../Data/USoilDataAsset.h"
#include "../Actors/ACropBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

USoilComponent::USoilComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentWaterLevel = 0.0f;
	bIsTilled = false;
	WaterEvaporationRate = 1.0f;
	EvaporationCheckInterval = 1.0f;
	TillProgress = 0.0f;
	TillThreshold = 0.0f;
}

void USoilComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USoilComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear timer if active
	if (GetWorld() && WaterEvaporationTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(WaterEvaporationTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void USoilComponent::Initialize(USoilDataAsset* InSoilData)
{
	if (!InSoilData)
	{
		UE_LOG(LogTemp, Warning, TEXT("USoilComponent::Initialize: SoilData is null!"));
		return;
	}

	SoilData = InSoilData;
	CurrentWaterLevel = 0.0f;
	bIsTilled = false;
	HeldCrop = nullptr;
}

float USoilComponent::GetEffectiveFertility() const
{
	if (!SoilData)
	{
		return 1.0f;
	}

	return SoilData->BaseFertility;
}

void USoilComponent::AddWater(float Amount)
{
	if (!SoilData || Amount <= 0.0f)
	{
		return;
	}

	float OldWaterLevel = CurrentWaterLevel;
	CurrentWaterLevel = FMath::Clamp(CurrentWaterLevel + Amount, 0.0f, SoilData->MaxWaterLevel);
	UE_LOG(LogTemp, Display, TEXT("Water level at %f"), CurrentWaterLevel);
	// Start evaporation timer if water was added and timer isn't running
	if (CurrentWaterLevel > 0.0f && !WaterEvaporationTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				WaterEvaporationTimerHandle,
				this,
				&USoilComponent::OnWaterEvaporationTimer,
				EvaporationCheckInterval,
				true
			);
		}
	}

	// Broadcast water level change
	if (FMath::Abs(CurrentWaterLevel - OldWaterLevel) > 0.01f)
	{
		OnWaterLevelChanged.Broadcast(GetOwner(), CurrentWaterLevel);
		UpdateVisuals();
	}
}

bool USoilComponent::ConsumeWater(float Amount)
{
	if (Amount <= 0.0f || CurrentWaterLevel <= 0.0f)
	{
		return false;
	}

	float OldWaterLevel = CurrentWaterLevel;
	CurrentWaterLevel = FMath::Max(0.0f, CurrentWaterLevel - Amount);

	// Stop evaporation timer if water is depleted
	if (CurrentWaterLevel <= 0.0f && WaterEvaporationTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(WaterEvaporationTimerHandle);
		}
	}

	// Broadcast water level change
	if (FMath::Abs(CurrentWaterLevel - OldWaterLevel) > 0.01f)
	{
		OnWaterLevelChanged.Broadcast(GetOwner(), CurrentWaterLevel);
		UpdateVisuals();
	}

	return true;
}

void USoilComponent::SetCrop(ACropBase* Crop)
{
	if (HeldCrop == Crop)
	{
		return;
	}

	HeldCrop = Crop;
	if (Crop)
	{
		OnCropPlanted.Broadcast(GetOwner(), Crop);
	}
}

bool USoilComponent::Till(const float TillPower)
{
	if (bIsTilled)
	{
		return true;
	}

	TillProgress += TillPower;
	if (TillProgress >= TillThreshold)
	{
		bIsTilled = true;
		OnSoilTilled.Broadcast(GetOwner());
		UpdateVisuals();
		return true;
	}
	return false;
}

bool USoilComponent::CanAcceptCrop() const
{
	return bIsTilled && HeldCrop == nullptr;
}

void USoilComponent::RemoveCrop()
{
	if (HeldCrop)
	{
		ACropBase* RemovedCrop = HeldCrop;
		HeldCrop = nullptr;
		OnCropRemoved.Broadcast(GetOwner());
	}
}

void USoilComponent::OnWaterEvaporationTimer()
{
	if (!SoilData || CurrentWaterLevel <= 0.0f)
	{
		// Stop timer if no water
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(WaterEvaporationTimerHandle);
		}
		return;
	}

	// Calculate evaporation based on retention multiplier
	float EvaporationAmount = WaterEvaporationRate / SoilData->WaterRetentionMultiplier;
	ConsumeWater(EvaporationAmount * EvaporationCheckInterval);
}

void USoilComponent::UpdateVisuals()
{
	// This will be implemented by the actor to update material parameters
	// Event-driven, not Tick-based
}

