#include "UCropGrowthComponent.h"
#include "../Data/UCropDataAsset.h"
#include "../Actors/ASoilPlot.h"
#include "../Components/USoilComponent.h"
#include "../Subsystems/UCropManagerSubsystem.h"
#include "Engine/World.h"

UCropGrowthComponent::UCropGrowthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentGrowthProgress = 0.0f;
	bIsWithered = false;
	bGrowthActive = false;
	WitherTimeWithoutWater = 30.0f;
	TimeWithoutWater = 0.0f;
	GrowthIncrementPerSecond = 0.01f;
	LastGrowthStageIndex = -1;
}

void UCropGrowthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCropGrowthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from crop manager if active
	if (bGrowthActive && GetWorld())
	{
		if (UCropManagerSubsystem* CropManager = GetWorld()->GetSubsystem<UCropManagerSubsystem>())
		{
			CropManager->UnregisterCrop(this);
		}
		bGrowthActive = false;
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

	// Unregister from manager if already registered
	if (bGrowthActive && GetWorld())
	{
		if (UCropManagerSubsystem* CropManager = GetWorld()->GetSubsystem<UCropManagerSubsystem>())
		{
			CropManager->UnregisterCrop(this);
		}
		bGrowthActive = false;
	}

	CropData = InCropData;
	ParentSoil = InParentSoil;
	CurrentGrowthProgress = 0.0f;
	bIsWithered = false;
	TimeWithoutWater = 0.0f;
	LastGrowthStageIndex = -1;

	// Set wither time from crop data asset
	WitherTimeWithoutWater = InCropData->WitherTimeWithoutWater;

	// Calculate growth increment per second based on growth time and soil fertility
	if (USoilComponent* SoilComp = ParentSoil->FindComponentByClass<USoilComponent>())
	{
		float EffectiveFertility = SoilComp->GetEffectiveFertility();
		float AdjustedGrowthTime = CropData->GrowthTimeSeconds / EffectiveFertility;
		GrowthIncrementPerSecond = 1.0f / AdjustedGrowthTime;
	}
	else
	{
		GrowthIncrementPerSecond = 1.0f / CropData->GrowthTimeSeconds;
	}
	StartGrowth();
}

void UCropGrowthComponent::StartGrowth()
{
	if (!CropData || !ParentSoil)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCropGrowthComponent::StartGrowth: CropData or ParentSoil is null!"));
		return;
	}

	// Register with crop manager
	if (UWorld* World = GetWorld())
	{
		if (UCropManagerSubsystem* CropManager = World->GetSubsystem<UCropManagerSubsystem>())
		{
			CropManager->RegisterCrop(this);
			bGrowthActive = true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UCropGrowthComponent::StartGrowth: CropManagerSubsystem not found!"));
		}
	}
	UpdateMesh();
}

void UCropGrowthComponent::PauseGrowth()
{
	// Unregister from crop manager
	if (bGrowthActive && GetWorld())
	{
		if (UCropManagerSubsystem* CropManager = GetWorld()->GetSubsystem<UCropManagerSubsystem>())
		{
			CropManager->UnregisterCrop(this);
		}
		bGrowthActive = false;
	}
}

void UCropGrowthComponent::UpdateGrowth(float DeltaTime)
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
		CurrentGrowthProgress = FMath::Min(1.0f, CurrentGrowthProgress + GrowthIncrementPerSecond * DeltaTime);

		// Consume water
		SoilComp->ConsumeWater(CropData->WaterConsumptionRate * DeltaTime);

		// Check if fully grown (reached 100% for the first time)
		if (CurrentGrowthProgress >= 1.0f && OldProgress < 1.0f)
		{
			OnCropFullyGrown.Broadcast(GetOwner());
			UpdateMesh();
		}
		// Update mesh if progress increased (stage might have changed)
		else if (CurrentGrowthProgress > OldProgress)
		{
			UpdateMesh();
		}
	}
	else
	{
		// No water - increment time without water
		TimeWithoutWater += DeltaTime;

		// Check if crop should wither
		if (TimeWithoutWater >= WitherTimeWithoutWater)
		{
			bIsWithered = true;
			OnCropWithered.Broadcast(GetOwner());

			// Unregister from manager (crop is withered, no longer needs updates)
			if (bGrowthActive && GetWorld())
			{
				if (UCropManagerSubsystem* CropManager = GetWorld()->GetSubsystem<UCropManagerSubsystem>())
				{
					CropManager->UnregisterCrop(this);
				}
				bGrowthActive = false;
			}
		}
	}
}

void UCropGrowthComponent::UpdateMesh()
{
	// Don't update growth stage mesh if crop is withered (withered mesh takes priority)
	if (bIsWithered)
	{
		return;
	}

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