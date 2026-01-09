#include "ACropBase.h"
#include "../Components/UCropGrowthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../Data/UCropDataAsset.h"
#include "../Actors/ASoilPlot.h"
#include "../Components/USoilComponent.h"
#include "../Data/USoilDataAsset.h"
#include "../Data/FHarvestResult.h"
#include "../Data/UItemDataAsset.h"
#include "../Actors/ItemPickup.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

ACropBase::ACropBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	GrowthComponent = CreateDefaultSubobject<UCropGrowthComponent>(TEXT("GrowthComponent"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
}

void ACropBase::BeginPlay()
{
	Super::BeginPlay();

	GrowthComponent->OnGrowthStageChanged.AddDynamic(this, &ACropBase::OnGrowthStageChanged);
	GrowthComponent->OnCropFullyGrown.AddDynamic(this, &ACropBase::OnCropFullyGrown);
	GrowthComponent->OnCropWithered.AddDynamic(this, &ACropBase::OnCropWithered);
}

void ACropBase::Initialize(UCropDataAsset* InCropData, ASoilPlot* InParentSoil)
{
	if (!InCropData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACropBase::Initialize: CropDataAsset is null!"));
		return;
	}

	if (!InParentSoil)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACropBase::Initialize: ParentSoil is null!"));
		return;
	}

	CropDataAsset = InCropData;
	ParentSoil = InParentSoil;

	GrowthComponent->OnGrowthStageChanged.Clear();
	GrowthComponent->OnCropFullyGrown.Clear();
	GrowthComponent->OnCropWithered.Clear();

	GrowthComponent->OnGrowthStageChanged.AddDynamic(this, &ACropBase::OnGrowthStageChanged);
	GrowthComponent->OnCropFullyGrown.AddDynamic(this, &ACropBase::OnCropFullyGrown);
	GrowthComponent->OnCropWithered.AddDynamic(this, &ACropBase::OnCropWithered);

	GrowthComponent->Initialize(InCropData, InParentSoil);
}

FHarvestResult ACropBase::Harvest_Implementation(AActor* Harvester, float ToolPower)
{
	FHarvestResult Result;

	if (!CanHarvest_Implementation())
	{
		Result.bSuccess = false;
		return Result;
	}

	if (!CropDataAsset || !ParentSoil)
	{
		Result.bSuccess = false;
		return Result;
	}

	HarvestProgress += ToolPower;
	if (HarvestProgress >= CropDataAsset->HarvestPowerNeeded)
	{
		bool bIsWithered = GrowthComponent && GrowthComponent->IsWithered();

		if (!bIsWithered)
		{
			int32 BaseQuantity = CropDataAsset->BaseHarvestQuantity;
			int32 FinalQuantity = BaseQuantity;

			if (USoilComponent* SoilComp = ParentSoil->FindComponentByClass<USoilComponent>())
			{
				if (USoilDataAsset* SoilData = SoilComp->GetSoilData())
				{
					float RandomValue = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);
					if (RandomValue <= SoilData->YieldChance)
					{
						FinalQuantity *= 2;
					}
				}
			}

			if (CropDataAsset->HarvestItem.IsValid())
			{
				UItemDataAsset* HarvestItem = CropDataAsset->HarvestItem.LoadSynchronous();
				if (HarvestItem)
				{
					Result.HarvestItem = HarvestItem;
					Result.Quantity = FinalQuantity;
					Result.bSuccess = true;

					SpawnHarvestItems(HarvestItem, FinalQuantity);
				}
			}

			if (CropDataAsset && GetWorld())
			{
				FVector SpawnLocation = GetActorLocation();
				SpawnLocation.Z += 10.0f; // Slightly above ground

				if (CropDataAsset->HarvestParticleEffect)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(),
						CropDataAsset->HarvestParticleEffect,
						SpawnLocation,
						FRotator::ZeroRotator,
						FVector::OneVector,
						true,
						true
					);
				}
				else if (CropDataAsset->HarvestParticleEffectCascade)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						CropDataAsset->HarvestParticleEffectCascade,
						SpawnLocation,
						FRotator::ZeroRotator,
						true
					);
				}
			}
		}
		else
		{
			Result.bSuccess = true;
		}

		if (USoilComponent* SoilComp = ParentSoil->FindComponentByClass<USoilComponent>())
		{
			SoilComp->RemoveCrop();
		}

		Destroy();

		return Result;
	}

	Result.bSuccess = false;
	return Result;
}

bool ACropBase::CanHarvest_Implementation() const
{
	if (!GrowthComponent || !CropDataAsset)
	{
		return false;
	}

	return (GrowthComponent->IsFullyGrown() && !GrowthComponent->IsWithered()) || GrowthComponent->IsWithered();
}

FText ACropBase::GetHarvestText_Implementation() const
{
	if (!CanHarvest_Implementation())
	{
		return FText::FromString("Not Ready");
	}

	if (GrowthComponent && GrowthComponent->IsWithered())
	{
		return FText::FromString("Remove Withered Crop");
	}

	return FText::FromString("Harvest");
}

FText ACropBase::GetTooltipText_Implementation() const
{
	return GetHarvestText_Implementation();
}

FVector ACropBase::GetActionLocation_Implementation() const
{
	FVector Location = GetActorLocation();
	if (MeshComponent)
	{
		FBoxSphereBounds Bounds = MeshComponent->CalcBounds(MeshComponent->GetComponentTransform());
		Location.Z = Bounds.Origin.Z + Bounds.BoxExtent.Z * 0.5f;
	}
	return Location;
}

float ACropBase::GetInteractionRange_Implementation() const
{
	return 100.0f;
}

void ACropBase::OnGrowthStageChanged(AActor* Crop, float Progress)
{
	if (!CropDataAsset)
	{
		return;
	}

	int8 MeshIndex;
	if (Progress >= 1.0f)
	{
		MeshIndex = 3;
	}
	else if (Progress >= 0.5f)
	{
		MeshIndex = 2;
	}
	else if (Progress >= 0.25f)
	{
		MeshIndex = 1;
	}
	else
	{
		MeshIndex = 0;
	}

	if (CropDataAsset->GrowthMeshes.IsValidIndex(MeshIndex) && CropDataAsset->GrowthMeshes[MeshIndex])
	{
		MeshComponent->SetStaticMesh(CropDataAsset->GrowthMeshes[MeshIndex]);
	}
}

void ACropBase::OnCropFullyGrown(AActor* Crop)
{
}

void ACropBase::OnCropWithered(AActor* Crop)
{
	if (!CropDataAsset)
	{
		return;
	}

	if (CropDataAsset->WitheredMesh)
	{
		MeshComponent->SetStaticMesh(CropDataAsset->WitheredMesh);
	}
}

void ACropBase::SpawnHarvestItems(UItemDataAsset* ItemData, int32 Quantity)
{
	if (!ItemData || !GetWorld() || Quantity <= 0)
	{
		return;
	}

	for (int32 i = 0; i < Quantity; ++i)
	{
		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Z += 50.0f;
		
		FVector RandomOffset = FVector(
			UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f),
			UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f),
			0.0f
		);
		SpawnLocation += RandomOffset;

		TSubclassOf<AActor> SpawnClass = ItemPickupClass;
		if (!SpawnClass)
		{
			SpawnClass = AItemPickup::StaticClass();
		}

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SpawnClass, SpawnLocation, FRotator::ZeroRotator);
		if (AItemPickup* ItemPickup = Cast<AItemPickup>(SpawnedActor))
		{
		}
	}
}

