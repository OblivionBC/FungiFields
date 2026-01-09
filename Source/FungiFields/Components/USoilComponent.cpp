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
	if (GetWorld() && WaterEvaporationTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(WaterEvaporationTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void USoilComponent::Initialize(USoilDataAsset* InSoilData)
{
	SoilData = InSoilData;
	CurrentWaterLevel = 0.0f;
	bIsTilled = false;
	HeldCrop = nullptr;
	TillProgress = 0.0f;
	
	if (InSoilData)
	{
		OnSoilStateChanged.Broadcast(GetOwner(), GetSoilState());
	}
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

	ESoilState OldState = GetSoilState();
	float OldWaterLevel = CurrentWaterLevel;
	CurrentWaterLevel = FMath::Clamp(CurrentWaterLevel + Amount, 0.0f, SoilData->MaxWaterLevel);
	UE_LOG(LogTemp, Display, TEXT("Water level at %f"), CurrentWaterLevel);
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

	if (FMath::Abs(CurrentWaterLevel - OldWaterLevel) > 0.01f)
	{
		OnWaterLevelChanged.Broadcast(GetOwner(), CurrentWaterLevel);
		
		ESoilState NewState = GetSoilState();
		if (OldState != NewState)
		{
			OnSoilStateChanged.Broadcast(GetOwner(), NewState);
		}
		
		UpdateVisuals();
	}
}

bool USoilComponent::ConsumeWater(float Amount)
{
	if (Amount <= 0.0f || CurrentWaterLevel <= 0.0f)
	{
		return false;
	}

	ESoilState OldState = GetSoilState();
	float OldWaterLevel = CurrentWaterLevel;
	CurrentWaterLevel = FMath::Max(0.0f, CurrentWaterLevel - Amount);

	if (CurrentWaterLevel <= 0.0f && WaterEvaporationTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(WaterEvaporationTimerHandle);
		}
	}

	if (FMath::Abs(CurrentWaterLevel - OldWaterLevel) > 0.01f)
	{
		OnWaterLevelChanged.Broadcast(GetOwner(), CurrentWaterLevel);
		
		ESoilState NewState = GetSoilState();
		if (OldState != NewState)
		{
			OnSoilStateChanged.Broadcast(GetOwner(), NewState);
		}
		
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
	if (!HasSoil())
	{
		return false;
	}

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
	return HasSoil() && bIsTilled && HeldCrop == nullptr;
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
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(WaterEvaporationTimerHandle);
		}
		return;
	}

	float EvaporationAmount = WaterEvaporationRate / SoilData->WaterRetentionMultiplier;
	ConsumeWater(EvaporationAmount * EvaporationCheckInterval);
}

void USoilComponent::SetSoilType(USoilDataAsset* InSoilData)
{
	if (!InSoilData)
	{
		return;
	}

	ESoilState OldState = GetSoilState();
	SoilData = InSoilData;
	CurrentWaterLevel = 0.0f;
	bIsTilled = false;
	TillProgress = 0.0f;
	
	ESoilState NewState = GetSoilState();
	if (OldState != NewState)
	{
		OnSoilStateChanged.Broadcast(GetOwner(), NewState);
	}
}

ESoilState USoilComponent::GetSoilState() const
{
	if (!HasSoil())
	{
		return ESoilState::Empty;
	}

	if (CurrentWaterLevel > 0.0f)
	{
		return ESoilState::Wet;
	}

	return ESoilState::Dry;
}

void USoilComponent::UpdateVisuals()
{
}

