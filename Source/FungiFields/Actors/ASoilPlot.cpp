#include "ASoilPlot.h"
#include "../Components/USoilComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../Data/USoilDataAsset.h"
#include "../Data/USoilContainerDataAsset.h"
#include "../Actors/ACropBase.h"
#include "../Data/UCropDataAsset.h"
#include "../ENUM/EToolType.h"
#include "../Interfaces/ITooltipProvider.h"
#include "../Interfaces/IHarvestableInterface.h"
#include "../Data/FHarvestResult.h"
#include "../Data/UToolDataAsset.h"
#include "../ENUM/ESoilState.h"
#include "../Data/UItemDataAsset.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

ASoilPlot::ASoilPlot()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SoilComponent = CreateDefaultSubobject<USoilComponent>(TEXT("SoilComponent"));

	ContainerMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerMeshComponent"));
	ContainerMeshComponent->SetupAttachment(RootComponent);

	SoilMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SoilMeshComponent"));
	SoilMeshComponent->SetupAttachment(RootComponent);

	CropSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CropSpawnPoint"));
	CropSpawnPoint->SetupAttachment(RootComponent);
	CropSpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	DynamicSoilMaterial = nullptr;
}

void ASoilPlot::BeginPlay()
{
	Super::BeginPlay();

	if (ContainerDataAsset)
	{
		Initialize(ContainerDataAsset, SoilDataAsset);
	}
	else if (SoilDataAsset)
	{
		// Backward compatibility: if only soil data is set, initialize with it
		Initialize(SoilDataAsset);
	}
}

void ASoilPlot::Initialize(USoilContainerDataAsset* InContainerData, USoilDataAsset* InSoilData)
{
	// Set container data and mesh
	if (InContainerData)
	{
		ContainerDataAsset = InContainerData;
		if (InContainerData->ContainerMesh && ContainerMeshComponent)
		{
			ContainerMeshComponent->SetStaticMesh(InContainerData->ContainerMesh);
		}
	}

	// Initialize soil (can be nullptr for empty plots)
	InitializeSoil(InSoilData);
}

void ASoilPlot::Initialize(USoilDataAsset* InSoilData)
{
	// This version only initializes soil, container should already be set
	InitializeSoil(InSoilData);
}

void ASoilPlot::InitializeSoil(USoilDataAsset* InSoilData)
{
	SoilDataAsset = InSoilData;
	SoilComponent->Initialize(InSoilData);

	// Subscribe to soil component delegates (only if not already subscribed)
	// RemoveDynamic will safely do nothing if not bound, so we can call it first to avoid duplicates
	SoilComponent->OnSoilTilled.RemoveDynamic(this, &ASoilPlot::OnSoilTilled);
	SoilComponent->OnSoilTilled.AddDynamic(this, &ASoilPlot::OnSoilTilled);
	
	SoilComponent->OnWaterLevelChanged.RemoveDynamic(this, &ASoilPlot::OnWaterLevelChanged);
	SoilComponent->OnWaterLevelChanged.AddDynamic(this, &ASoilPlot::OnWaterLevelChanged);
	
	SoilComponent->OnCropPlanted.RemoveDynamic(this, &ASoilPlot::OnCropPlanted);
	SoilComponent->OnCropPlanted.AddDynamic(this, &ASoilPlot::OnCropPlanted);
	
	SoilComponent->OnCropRemoved.RemoveDynamic(this, &ASoilPlot::OnCropRemoved);
	SoilComponent->OnCropRemoved.AddDynamic(this, &ASoilPlot::OnCropRemoved);
	
	SoilComponent->OnSoilStateChanged.RemoveDynamic(this, &ASoilPlot::OnSoilStateChanged);
	SoilComponent->OnSoilStateChanged.AddDynamic(this, &ASoilPlot::OnSoilStateChanged);

	if (InSoilData)
	{
		// Create dynamic material instance from soil material
		if (InSoilData->SoilMaterial && SoilMeshComponent)
		{
			DynamicSoilMaterial = UMaterialInstanceDynamic::Create(InSoilData->SoilMaterial, this);
			if (DynamicSoilMaterial)
			{
				SoilMeshComponent->SetMaterial(0, DynamicSoilMaterial);
				// Initialize wetness to 0
				DynamicSoilMaterial->SetScalarParameterValue(TEXT("Wetness"), 0.0f);
			}
		}

		// Show soil mesh
		if (SoilMeshComponent)
		{
			SoilMeshComponent->SetVisibility(true);
		}
	}
	else
	{
		// Hide soil mesh for empty plots
		if (SoilMeshComponent)
		{
			SoilMeshComponent->SetVisibility(false);
		}
		DynamicSoilMaterial = nullptr;
	}

	UpdateVisuals();
}

bool ASoilPlot::InteractTool_Implementation(EToolType ToolType, AActor* Interactor, float ToolPower)
{
	if (!SoilComponent)
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
		// Till the soil (only if soil is present)
		if (SoilComponent->HasSoil() && SoilComponent->Till(ToolPower))
		{
			// Set material parameter for tilled state
			if (DynamicSoilMaterial)
			{
				DynamicSoilMaterial->SetScalarParameterValue(TEXT("IsTilled"), 1.0f);
			}
			bSuccess = true;
			// Note: Soil tilled event is broadcast from UFarmingComponent::ExecuteFarmingAction
		}
		break;

	case EToolType::WateringCan:
		// Add water to soil (only if tilled and soil is present)
		if (SoilComponent->HasSoil() && SoilComponent->IsTilled())
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

	// Check if plot is empty
	if (!SoilComponent->HasSoil())
	{
		return FText::FromString("Empty Plot");
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
		// Hoe can only till untilled soil that has soil present
		return SoilComponent->HasSoil() && !SoilComponent->IsTilled();

	case EToolType::WateringCan:
		// Watering can can only water tilled soil that has soil present
		return SoilComponent->HasSoil() && SoilComponent->IsTilled();

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
	if (ContainerMeshComponent)
	{
		FBoxSphereBounds Bounds = ContainerMeshComponent->CalcBounds(ContainerMeshComponent->GetComponentTransform());
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
	if (!SoilComponent || !SoilMeshComponent)
	{
		return;
	}

	// Hide soil mesh if no soil is present
	if (!SoilComponent->HasSoil())
	{
		SoilMeshComponent->SetVisibility(false);
		DynamicSoilMaterial = nullptr;
		return;
	}

	// Show soil mesh
	SoilMeshComponent->SetVisibility(true);

	if (!SoilDataAsset || !DynamicSoilMaterial)
	{
		return;
	}

	// Calculate wetness amount (0.0 to 1.0 based on water level)
	float WetAmount = 0.0f;
	if (SoilDataAsset->MaxWaterLevel > 0.0f)
	{
		float CurrentWaterLevel = SoilComponent->GetWaterLevel();
		WetAmount = FMath::Clamp(CurrentWaterLevel / SoilDataAsset->MaxWaterLevel, 0.0f, 1.0f);
	}

	// Set wetness parameter
	DynamicSoilMaterial->SetScalarParameterValue(TEXT("Wetness"), WetAmount);

	// Set tilled parameter
	if (SoilComponent->IsTilled())
	{
		DynamicSoilMaterial->SetScalarParameterValue(TEXT("IsTilled"), 1.0f);
	}
	else
	{
		DynamicSoilMaterial->SetScalarParameterValue(TEXT("IsTilled"), 0.0f);
	}
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

void ASoilPlot::OnSoilStateChanged(AActor* Soil, ESoilState NewState)
{
	UpdateVisuals();
}

bool ASoilPlot::CanAcceptSoilBag_Implementation(UItemDataAsset* SoilBagItem) const
{
	if (!SoilBagItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASoilPlot::CanAcceptSoilBag: SoilBagItem is null"));
		return false;
	}
	
	if (!SoilBagItem->bIsSoilBag)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASoilPlot::CanAcceptSoilBag: Item is not a soil bag"));
		return false;
	}
	
	if (!SoilBagItem->SoilBagSoilDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASoilPlot::CanAcceptSoilBag: SoilBagSoilDataAsset is null"));
		return false;
	}

	if (!SoilComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASoilPlot::CanAcceptSoilBag: SoilComponent is null"));
		return false;
	}

	bool bHasSoil = SoilComponent->HasSoil();
	UE_LOG(LogTemp, Log, TEXT("ASoilPlot::CanAcceptSoilBag: HasSoil = %d"), bHasSoil);
	
	// Can only accept soil bag if plot is empty (no soil data asset)
	return !bHasSoil;
}

bool ASoilPlot::AddSoilFromBag_Implementation(UItemDataAsset* SoilBagItem)
{
	UE_LOG(LogTemp, Log, TEXT("ASoilPlot::AddSoilFromBag called"));
	
	if (!CanAcceptSoilBag_Implementation(SoilBagItem))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASoilPlot::AddSoilFromBag: CanAcceptSoilBag returned false"));
		return false;
	}
	
	if (!SoilBagItem->SoilBagSoilDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASoilPlot::AddSoilFromBag: SoilBagSoilDataAsset is null"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ASoilPlot::AddSoilFromBag: Initializing with soil data asset: %s"), *SoilBagItem->SoilBagSoilDataAsset->GetName());
	
	// Initialize the plot with the soil data from the bag
	Initialize(SoilBagItem->SoilBagSoilDataAsset);
	return true;
}

