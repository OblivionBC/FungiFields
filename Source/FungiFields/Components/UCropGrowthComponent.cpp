#include "UCropGrowthComponent.h"
#include "../Data/UCropDataAsset.h"
#include "../Actors/ASoilPlot.h"
#include "../Components/USoilComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCropGrowthComponent::UCropGrowthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentGrowthProgress = 0.0f;
	bIsWithered = false;
	WitherTimeWithoutWater = 30.0f;
	TimeWithoutWater = 0.0f;
	GrowthCheckInterval = 1.0f;
	GrowthIncrementPerCheck = 0.01f;
	LastGrowthStageIndex = -1;
}

void UCropGrowthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCropGrowthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear timer if active
	if (GetWorld() && GrowthTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(GrowthTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UCropGrowthComponent::Initialize(UCropDataAsset* InCropData, ASoilPlot* InParentSoil)
{
	if (!InCropData)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCropGrowthComponent::Initialize: CropData is null!"));
		return;
	}

	if (!InParentSoil)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCropGrowthComponent::Initialize: ParentSoil is null!"));
		return;
	}

	CropData = InCropData;
	ParentSoil = InParentSoil;
	CurrentGrowthProgress = 0.0f;
	bIsWithered = false;
	TimeWithoutWater = 0.0f;
	LastGrowthStageIndex = -1;

	// Calculate growth increment based on growth time and soil fertility
	if (USoilComponent* SoilComp = ParentSoil->FindComponentByClass<USoilComponent>())
	{
		float EffectiveFertility = SoilComp->GetEffectiveFertility();
		float AdjustedGrowthTime = CropData->GrowthTimeSeconds / EffectiveFertility;
		GrowthIncrementPerCheck = GrowthCheckInterval / AdjustedGrowthTime;
	}
	else
	{
		GrowthIncrementPerCheck = GrowthCheckInterval / CropData->GrowthTimeSeconds;
	}
}

void UCropGrowthComponent::StartGrowth()
{
	if (!CropData || !ParentSoil)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCropGrowthComponent::StartGrowth: CropData or ParentSoil is null!"));
		return;
	}

	// Start growth timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			GrowthTimerHandle,
			this,
			&UCropGrowthComponent::OnGrowthTimer,
			GrowthCheckInterval,
			true
		);
	}
}

void UCropGrowthComponent::PauseGrowth()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().PauseTimer(GrowthTimerHandle);
	}
}

void UCropGrowthComponent::OnGrowthTimer()
{
	if (!CropData || !ParentSoil || bIsWithered)
	{
		return;
	}

	// Check if soil has water
	USoilComponent* SoilComp = ParentSoil->FindComponentByClass<USoilComponent>();
	if (!SoilComp)
	{
		return;
	}

	bool bHasWater = SoilComp->HasWater();

	if (bHasWater)
	{
		// Reset time without water
		TimeWithoutWater = 0.0f;

		// Increment growth progress
		float OldProgress = CurrentGrowthProgress;
		CurrentGrowthProgress = FMath::Min(1.0f, CurrentGrowthProgress + GrowthIncrementPerCheck);

		// Consume water
		SoilComp->ConsumeWater(CropData->WaterConsumptionRate * GrowthCheckInterval);

		// Check if fully grown
		if (CurrentGrowthProgress >= 1.0f && OldProgress < 1.0f)
		{
			OnCropFullyGrown.Broadcast(GetOwner());
			UpdateMesh();
		}
		// Check if growth stage changed
		else if (CurrentGrowthProgress > OldProgress)
		{
			UpdateMesh();
		}
	}
	else
	{
		// No water - increment time without water
		TimeWithoutWater += GrowthCheckInterval;

		// Check if crop should wither
		if (TimeWithoutWater >= WitherTimeWithoutWater)
		{
			bIsWithered = true;
			OnCropWithered.Broadcast(GetOwner());
			UpdateMesh();

			// Pause growth timer
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().PauseTimer(GrowthTimerHandle);
			}
		}
	}
}

void UCropGrowthComponent::UpdateMesh()
{
	// Calculate current growth stage index (0, 1, 2, 3 for 0%, 25%, 50%, 100%)
	int32 CurrentStageIndex = -1;

	if (CurrentGrowthProgress >= 1.0f)
	{
		CurrentStageIndex = 3; // 100%
	}
	else if (CurrentGrowthProgress >= 0.5f)
	{
		CurrentStageIndex = 2; // 50%
	}
	else if (CurrentGrowthProgress >= 0.25f)
	{
		CurrentStageIndex = 1; // 25%
	}
	else
	{
		CurrentStageIndex = 0; // 0%
	}

	// Broadcast if stage changed
	if (CurrentStageIndex != LastGrowthStageIndex)
	{
		LastGrowthStageIndex = CurrentStageIndex;
		OnGrowthStageChanged.Broadcast(GetOwner(), CurrentGrowthProgress);
	}
}