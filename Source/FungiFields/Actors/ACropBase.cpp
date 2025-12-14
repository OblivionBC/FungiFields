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

	// Clear any existing bindings to ensure clean state
	GrowthComponent->OnGrowthStageChanged.Clear();
	GrowthComponent->OnCropFullyGrown.Clear();
	GrowthComponent->OnCropWithered.Clear();

	// Ensure subscription is set up before starting growth (BeginPlay may not have run yet)
	// This prevents missing delegate broadcasts if the timer fires before BeginPlay
	GrowthComponent->OnGrowthStageChanged.AddDynamic(this, &ACropBase::OnGrowthStageChanged);
	GrowthComponent->OnCropFullyGrown.AddDynamic(this, &ACropBase::OnCropFullyGrown);
	GrowthComponent->OnCropWithered.AddDynamic(this, &ACropBase::OnCropWithered);

	// Initialize growth component
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
		// Check if crop is withered
		bool bIsWithered = GrowthComponent && GrowthComponent->IsWithered();

		// Only spawn items and particles if crop is NOT withered
		if (!bIsWithered)
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
		}
		else
		{
			// Withered crop: mark as success but don't spawn items
			Result.bSuccess = true;
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

	// Can harvest if fully grown OR if withered
	return (GrowthComponent->IsFullyGrown() && !GrowthComponent->IsWithered()) || GrowthComponent->IsWithered();
}

FText ACropBase::GetHarvestText_Implementation() const
{
	if (!CanHarvest_Implementation())
	{
		return FText::FromString("Not Ready");
	}

	// Return different text if crop is withered
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
	int8 MeshIndex;
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
	if (CropDataAsset->GrowthMeshes.IsValidIndex(MeshIndex) && CropDataAsset->GrowthMeshes[MeshIndex])
	{
		MeshComponent->SetStaticMesh(CropDataAsset->GrowthMeshes[MeshIndex]);
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

