#include "ASoilPlot.h"
#include "../Components/USoilComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../Data/USoilDataAsset.h"
#include "../Actors/ACropBase.h"
#include "../Data/UCropDataAsset.h"
#include "../ENUM/EToolType.h"
#include "Engine/World.h"

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
	if (InSoilData->SoilMesh.IsValid())
	{
		UStaticMesh* LoadedMesh = InSoilData->SoilMesh.LoadSynchronous();
		if (LoadedMesh)
		{
			MeshComponent->SetStaticMesh(LoadedMesh);
		}
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

	switch (ToolType)
	{
	case EToolType::Hoe:
		// Till the soil
		if (SoilComponent->Till(ToolPower))
		{
			// Update mesh to tilled version if it was successfully tilled
			if (UStaticMesh* LoadedMesh = SoilDataAsset->TilledMesh.LoadSynchronous())
			{
					MeshComponent->SetStaticMesh(LoadedMesh);
			}
			return true;
		}
		break;

	case EToolType::WateringCan:
		// Add water to soil
		SoilComponent->AddWater(ToolPower);
		return true;

	default:
		break;
	}

	return false;
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

