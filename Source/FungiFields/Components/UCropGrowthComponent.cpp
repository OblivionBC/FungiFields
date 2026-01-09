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

	WitherTimeWithoutWater = InCropData->WitherTimeWithoutWater;

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

	USoilComponent* SoilComp = ParentSoil->FindComponentByClass<USoilComponent>();
	if (!SoilComp)
	{
		return;
	}

	bool bHasWater = SoilComp->HasWater();

	if (bHasWater)
	{
		TimeWithoutWater = 0.0f;

		float OldProgress = CurrentGrowthProgress;
		CurrentGrowthProgress = FMath::Min(1.0f, CurrentGrowthProgress + GrowthIncrementPerSecond * DeltaTime);

		SoilComp->ConsumeWater(CropData->WaterConsumptionRate * DeltaTime);

		if (CurrentGrowthProgress >= 1.0f && OldProgress < 1.0f)
		{
			OnCropFullyGrown.Broadcast(GetOwner());
			UpdateMesh();
		}
		else if (CurrentGrowthProgress > OldProgress)
		{
			UpdateMesh();
		}
	}
	else
	{
		TimeWithoutWater += DeltaTime;

		if (TimeWithoutWater >= WitherTimeWithoutWater)
		{
			bIsWithered = true;
			OnCropWithered.Broadcast(GetOwner());

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
	if (bIsWithered)
	{
		return;
	}

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
		CurrentStageIndex = 0;
	}

	if (CurrentStageIndex != LastGrowthStageIndex)
	{
		LastGrowthStageIndex = CurrentStageIndex;
		OnGrowthStageChanged.Broadcast(GetOwner(), CurrentGrowthProgress);
	}
}