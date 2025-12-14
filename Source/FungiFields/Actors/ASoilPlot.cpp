#include "ASoilPlot.h"
#include "../Components/USoilComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../Data/USoilDataAsset.h"
#include "../Actors/ACropBase.h"
#include "../Data/UCropDataAsset.h"
#include "../ENUM/EToolType.h"
#include "../Interfaces/ITooltipProvider.h"
#include "../Interfaces/IHarvestableInterface.h"
#include "../Data/FHarvestResult.h"
#include "../Data/UToolDataAsset.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

ASoilPlot::ASoilPlot()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SoilComponent = CreateDefaultSubobject<USoilComponent>(TEXT("SoilComponent"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	CropSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CropSpawnPoint"));
	CropSpawnPoint->SetupAttachment(RootComponent);
	CropSpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
}

void ASoilPlot::BeginPlay()
{
	Super::BeginPlay();

	if (SoilDataAsset)
	{
		Initialize(SoilDataAsset);
	}
}

void ASoilPlot::Initialize(USoilDataAsset* InSoilData)
{
	if (!InSoilData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASoilPlot::Initialize: SoilDataAsset is null!"));
		return;
	}

	SoilDataAsset = InSoilData;
	SoilComponent->Initialize(InSoilData);

	// Load and set mesh
	if (InSoilData->SoilMesh) {
		MeshComponent->SetStaticMesh(InSoilData->SoilMesh);
	}

	// Subscribe to soil component delegates
	SoilComponent->OnSoilTilled.AddDynamic(this, &ASoilPlot::OnSoilTilled);
	SoilComponent->OnWaterLevelChanged.AddDynamic(this, &ASoilPlot::OnWaterLevelChanged);
	SoilComponent->OnCropPlanted.AddDynamic(this, &ASoilPlot::OnCropPlanted);
	SoilComponent->OnCropRemoved.AddDynamic(this, &ASoilPlot::OnCropRemoved);
}

bool ASoilPlot::InteractTool_Implementation(EToolType ToolType, AActor* Interactor, float ToolPower)
{
	if (!SoilComponent || !SoilDataAsset)
	{
		return false;
	}

	// Validate the action before executing
	if (!CanInteractWithTool_Implementation(ToolType, Interactor))
	{
		return false;
	}

	bool bSuccess = false;
	UNiagaraSystem* ParticleEffect = nullptr;
	UParticleSystem* ParticleEffectCascade = nullptr;

	switch (ToolType)
	{
	case EToolType::Hoe:
		// Till the soil
		if (SoilComponent->Till(ToolPower))
		{
			// Update mesh to tilled version if it was successfully tilled
			if (SoilDataAsset->TilledMesh)
			{
				MeshComponent->SetStaticMesh(SoilDataAsset->TilledMesh);
			}
			bSuccess = true;
			// Note: Soil tilled event is broadcast from UFarmingComponent::ExecuteFarmingAction
		}
		break;

	case EToolType::WateringCan:
		// Add water to soil (only if tilled)
		if (SoilComponent->IsTilled())
		{
			SoilComponent->AddWater(ToolPower);
			bSuccess = true;
			// Note: Soil watered event is broadcast from UFarmingComponent::ExecuteFarmingAction
		}
		else
		{
			return false;
		}
		break;

	case EToolType::Scythe:
		// Harvest crop if present
		if (ACropBase* Crop = SoilComponent->GetCrop())
		{
			if (Crop->Implements<UHarvestableInterface>())
			{
				if (IHarvestableInterface::Execute_CanHarvest(Crop))
				{
					FHarvestResult HarvestResult = IHarvestableInterface::Execute_Harvest(Crop, Interactor, ToolPower);
					bSuccess = HarvestResult.bSuccess;
				}
			}
		}
		break;

	default:
		return false;
	}

	// Only spawn particles if action was successful
	if (bSuccess && GetWorld())
	{
		// Get particle effect from tool if available
		if (Interactor)
		{
			if (UInventoryComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryComponent>())
			{
				int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
				if (EquippedSlotIndex != INDEX_NONE)
				{
					const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
					if (Slots.IsValidIndex(EquippedSlotIndex))
					{
						const FInventorySlot& Slot = Slots[EquippedSlotIndex];
						if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(Slot.ItemDefinition))
						{
							ParticleEffect = ToolData->ToolParticleEffect;
							ParticleEffectCascade = ToolData->ToolParticleEffectCascade;
						}
					}
				}
			}
		}

		// Spawn particles if available
		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Z += 10.0f; // Slightly above ground

		if (ParticleEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				ParticleEffect,
				SpawnLocation,
				FRotator::ZeroRotator,
				FVector::OneVector,
				true,
				true
			);
		}
		else if (ParticleEffectCascade)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ParticleEffectCascade,
				SpawnLocation,
				FRotator::ZeroRotator,
				true
			);
		}
	}

	return bSuccess;
}

bool ASoilPlot::CanAcceptSeed_Implementation() const
{
	if (!SoilComponent)
	{
		return false;
	}

	return SoilComponent->CanAcceptCrop();
}

bool ASoilPlot::PlantSeed_Implementation(UCropDataAsset* CropToPlant, AActor* Planter)
{
	if (!CropToPlant || !SoilComponent || !CanAcceptSeed_Implementation())
	{
		return false;
	}

	// Spawn crop
	ACropBase* NewCrop = SpawnCrop(CropToPlant);
	if (NewCrop)
	{
		SoilComponent->SetCrop(NewCrop);
		return true;
	}

	return false;
}

FText ASoilPlot::GetInteractionText_Implementation() const
{
	if (!SoilComponent)
	{
		return FText::FromString("Soil");
	}

	if (!SoilComponent->IsTilled())
	{
		return FText::FromString("Till Soil");
	}

	if (SoilComponent->GetCrop())
	{
		return FText::FromString("Remove Crop");
	}

	return FText::FromString("Plant Seed");
}

bool ASoilPlot::CanInteractWithTool_Implementation(EToolType ToolType, AActor* Interactor) const
{
	if (!SoilComponent)
	{
		return false;
	}

	switch (ToolType)
	{
	case EToolType::Hoe:
		// Hoe can only till untilled soil
		return !SoilComponent->IsTilled();

	case EToolType::WateringCan:
		// Watering can can only water tilled soil
		return SoilComponent->IsTilled();

	case EToolType::Scythe:
		// Scythe can only harvest if there's a harvestable crop
		if (ACropBase* Crop = SoilComponent->GetCrop())
		{
			if (Crop->Implements<UHarvestableInterface>())
			{
				return IHarvestableInterface::Execute_CanHarvest(Crop);
			}
		}
		return false;

	default:
		return false;
	}
}

FText ASoilPlot::GetTooltipText_Implementation() const
{
	return GetInteractionText_Implementation();
}

FVector ASoilPlot::GetActionLocation_Implementation() const
{
	// Return the center of the soil plot, slightly above ground
	FVector Location = GetActorLocation();
	if (MeshComponent)
	{
		FBoxSphereBounds Bounds = MeshComponent->CalcBounds(MeshComponent->GetComponentTransform());
		Location.Z = Bounds.Origin.Z + Bounds.BoxExtent.Z * 0.5f;
	}
	return Location;
}

float ASoilPlot::GetInteractionRange_Implementation() const
{
	// Default interaction range for soil plots
	return 150.0f;
}

ACropBase* ASoilPlot::SpawnCrop(UCropDataAsset* CropData)
{
	if (!CropData || !CropActorClass || !GetWorld())
	{
		return nullptr;
	}

	// Spawn crop at spawn point
	FVector SpawnLocation = CropSpawnPoint->GetComponentLocation();
	FRotator SpawnRotation = CropSpawnPoint->GetComponentRotation();

	ACropBase* NewCrop = GetWorld()->SpawnActor<ACropBase>(CropActorClass, SpawnLocation, SpawnRotation);
	if (NewCrop)
	{
		// Initialize crop with data and parent soil
		NewCrop->Initialize(CropData, this);
	}

	return NewCrop;
}

void ASoilPlot::UpdateVisuals()
{
	// Update material parameters based on water level
}

void ASoilPlot::OnSoilTilled(AActor* Soil)
{
	UpdateVisuals();
}

void ASoilPlot::OnCropPlanted(AActor* Soil, ACropBase* Crop)
{
	UpdateVisuals();
}

void ASoilPlot::OnCropRemoved(AActor* Soil)
{
	UpdateVisuals();
}

void ASoilPlot::OnWaterLevelChanged(AActor* Soil, float NewWaterLevel)
{
	UpdateVisuals();
}

