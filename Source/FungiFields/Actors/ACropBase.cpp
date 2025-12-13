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
#include "../Interfaces/ITooltipProvider.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"
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

	// Subscribe to growth component delegates
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

	// Initialize growth component
	GrowthComponent->Initialize(InCropData, InParentSoil);

	// Load and set initial mesh (0% growth)
	if (InCropData->GrowthMeshes.Num() > 0)
	{
		UStaticMesh* LoadedMesh = InCropData->GrowthMeshes[0].LoadSynchronous();
		if (LoadedMesh)
		{
			MeshComponent->SetStaticMesh(LoadedMesh);
		}
	}

	GrowthComponent->StartGrowth();
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
	if (HarvestProgress >= HarvestThreshold)
	{
		// Calculate yield
		int32 BaseQuantity = CropDataAsset->BaseHarvestQuantity;
		int32 FinalQuantity = BaseQuantity;

		// Check for yield bonus from soil
		if (USoilComponent* SoilComp = ParentSoil->FindComponentByClass<USoilComponent>())
		{
			if (USoilDataAsset* SoilData = SoilComp->GetSoilData())
			{
				// Roll for double yield based on soil yield chance
				float RandomValue = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);
				if (RandomValue <= SoilData->YieldChance)
				{
					FinalQuantity *= 2;
				}
			}
		}

		// Load harvest item
		if (CropDataAsset->HarvestItem.IsValid())
		{
			UItemDataAsset* HarvestItem = CropDataAsset->HarvestItem.LoadSynchronous();
			if (HarvestItem)
			{
				Result.HarvestItem = HarvestItem;
				Result.Quantity = FinalQuantity;
				Result.bSuccess = true;

				// Spawn harvest items
				SpawnHarvestItems(HarvestItem, FinalQuantity);
			}
		}

		// Spawn harvest particles before destroying
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

		// Remove crop from soil
		if (USoilComponent* SoilComp = ParentSoil->FindComponentByClass<USoilComponent>())
		{
			SoilComp->RemoveCrop();
		}

		// Destroy crop actor
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

	return GrowthComponent->IsFullyGrown() && !GrowthComponent->IsWithered();
}

FText ACropBase::GetHarvestText_Implementation() const
{
	if (!CanHarvest_Implementation())
	{
		return FText::FromString("Not Ready");
	}

	return FText::FromString("Harvest");
}

FText ACropBase::GetTooltipText_Implementation() const
{
	return GetHarvestText_Implementation();
}

FVector ACropBase::GetActionLocation_Implementation() const
{
	// Return the center of the crop, slightly above ground
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
	// Default interaction range for crops
	return 100.0f;
}

void ACropBase::OnGrowthStageChanged(AActor* Crop, float Progress)
{
	if (!CropDataAsset)
	{
		return;
	}

	// Determine which mesh to use based on progress
	int32 MeshIndex = -1;
	if (Progress >= 1.0f)
	{
		MeshIndex = 3; // 100%
	}
	else if (Progress >= 0.5f)
	{
		MeshIndex = 2; // 50%
	}
	else if (Progress >= 0.25f)
	{
		MeshIndex = 1; // 25%
	}
	else
	{
		MeshIndex = 0; // 0%
	}

	// Update mesh if valid index
	if (CropDataAsset->GrowthMeshes.IsValidIndex(MeshIndex) && CropDataAsset->GrowthMeshes[MeshIndex].IsValid())
	{
		UStaticMesh* LoadedMesh = CropDataAsset->GrowthMeshes[MeshIndex].LoadSynchronous();
		if (LoadedMesh)
		{
			MeshComponent->SetStaticMesh(LoadedMesh);
		}
	}
}

void ACropBase::OnCropFullyGrown(AActor* Crop)
{
	// Crop is fully grown and ready for harvest
}

void ACropBase::OnCropWithered(AActor* Crop)
{
	if (!CropDataAsset)
	{
		return;
	}

	// Update mesh to withered version
	if (CropDataAsset->WitheredMesh.IsValid())
	{
		UStaticMesh* LoadedMesh = CropDataAsset->WitheredMesh.LoadSynchronous();
		if (LoadedMesh)
		{
			MeshComponent->SetStaticMesh(LoadedMesh);
		}
	}
}

void ACropBase::SpawnHarvestItems(UItemDataAsset* ItemData, int32 Quantity)
{
	if (!ItemData || !GetWorld() || Quantity <= 0)
	{
		return;
	}

	// Spawn item pickups
	for (int32 i = 0; i < Quantity; ++i)
	{
		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Z += 50.0f; // Slightly above ground
		
		// Add some random offset to spread items
		FVector RandomOffset = FVector(
			UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f),
			UKismetMathLibrary::RandomFloatInRange(-30.0f, 30.0f),
			0.0f
		);
		SpawnLocation += RandomOffset;

		// Spawn item pickup if class is set, otherwise use default ItemPickup
		TSubclassOf<AActor> SpawnClass = ItemPickupClass;
		if (!SpawnClass)
		{
			SpawnClass = AItemPickup::StaticClass();
		}

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SpawnClass, SpawnLocation, FRotator::ZeroRotator);
		if (AItemPickup* ItemPickup = Cast<AItemPickup>(SpawnedActor))
		{
			// Set item data on pickup
		}
	}
}

