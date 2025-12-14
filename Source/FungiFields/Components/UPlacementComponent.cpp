#include "UPlacementComponent.h"
#include "Camera/CameraComponent.h"
#include "../Data/UItemDataAsset.h"
#include "../Data/USoilDataAsset.h"
#include "../Actors/ASoilPlot.h"
#include "../Components/InventoryComponent.h"
#include "../Widgets/InteractionWidget.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "InputActionValue.h"
#include "USoilComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInterface.h"
#include "Blueprint/UserWidget.h"

UPlacementComponent::UPlacementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	bIsInPlacementMode = false;
	CurrentPlaceableItem = nullptr;
	PreviewActor = nullptr;
	PreviewLocation = FVector::ZeroVector;
	PreviewRotation = FRotator::ZeroRotator;
	bPreviewLocationValid = false;
	GroundTraceDistance = 1000.0f;
	MinPlacementDistance = 50.0f;
	PlacementCheckRadius = 50.0f;
	RotationAdjustmentStep = 15.0f;
	CurrentRotationOffset = 0.0f;
}

void UPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsInPlacementMode)
	{
		UpdatePreview();
	}
}

void UPlacementComponent::SetCamera(UCameraComponent* Camera)
{
	CameraComponent = Camera;
}

void UPlacementComponent::EnterPlacementMode(UItemDataAsset* PlaceableItem)
{
	if (!PlaceableItem || !PlaceableItem->bIsPlaceable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPlacementComponent::EnterPlacementMode: Item is not placeable!"));
		return;
	}

	// Check if PlaceableActorClass is set
	// For soil plots, PlaceableSoilDataAsset should also be set, but for other placeables it might be null
	if (PlaceableItem->PlaceableActorClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPlacementComponent::EnterPlacementMode: PlaceableActorClass is null!"));
		return;
	}

	CurrentPlaceableItem = PlaceableItem;
	bIsInPlacementMode = true;
	CurrentRotationOffset = 0.0f;
	
	UpdatePreviewActor();
	UpdatePreview(); // Update preview immediately so it's visible right away
	ShowPlacementInstructions();
}

void UPlacementComponent::ExitPlacementMode()
{
	bIsInPlacementMode = false;
	CurrentPlaceableItem = nullptr;
	DestroyPreviewActor();
	HidePlacementInstructions();
}

void UPlacementComponent::PlaceItem(const FInputActionValue& Value)
{
	if (!bIsInPlacementMode || !bPreviewLocationValid)
	{
		return;
	}

	PlaceItemAtLocation(PreviewLocation, PreviewRotation);
}

void UPlacementComponent::AdjustPlacementRotation(const FInputActionValue& Value)
{
	if (!bIsInPlacementMode)
	{
		return;
	}

	// Get scroll delta (positive for scroll up, negative for scroll down)
	float ScrollDelta = Value.Get<float>();
	
	// Adjust rotation based on scroll direction
	CurrentRotationOffset += (ScrollDelta * RotationAdjustmentStep);
	
	// Keep rotation in 0-360 range
	while (CurrentRotationOffset >= 360.0f)
	{
		CurrentRotationOffset -= 360.0f;
	}
	while (CurrentRotationOffset < 0.0f)
	{
		CurrentRotationOffset += 360.0f;
	}
}

void UPlacementComponent::PickupItem(const FInputActionValue& Value)
{
	if (!CameraComponent)
	{
		return;
	}

	FHitResult HitResult;
	FVector Start = CameraComponent->GetComponentLocation();
	FVector ForwardVector = CameraComponent->GetForwardVector();
	FVector End = Start + (ForwardVector * GroundTraceDistance);

	FCollisionQueryParams TraceParams(FName(TEXT("PickupTrace")), true, GetOwner());
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		TraceParams
	);

	if (bHit && HitResult.GetActor())
	{
		ASoilPlot* HitSoilPlot = Cast<ASoilPlot>(HitResult.GetActor());
		if (HitSoilPlot)
		{
			// Get the soil data asset from the soil plot
			USoilDataAsset* SoilData = HitSoilPlot->GetSoilDataAsset();
			if (!SoilData && HitSoilPlot->GetSoilComponent())
			{
				SoilData = HitSoilPlot->GetSoilComponent()->GetSoilData();
			}

			if (SoilData)
			{
				// Broadcast delegate - let subscribers handle adding item to inventory
				OnPlaceablePickedUp.Broadcast(GetOwner(), SoilData);
				
				// Destroy the placeable actor
				HitSoilPlot->Destroy();
			}
		}
	}
}

void UPlacementComponent::UpdatePreview()
{
	if (!CameraComponent || !CurrentPlaceableItem)
	{
		return;
	}

	FHitResult HitResult;
	if (!PerformGroundTrace(HitResult))
	{
		bPreviewLocationValid = false;
		if (PreviewActor)
		{
			PreviewActor->SetActorHiddenInGame(true);
		}
		return;
	}

	// Calculate placement location and rotation
	FVector HitLocation = HitResult.Location;
	FVector HitNormal = HitResult.Normal;
	
	// Apply offset from item data asset
	float OffsetZ = CurrentPlaceableItem->PreviewOffsetZ;
	HitLocation.Z += OffsetZ;

	// Calculate rotation from normal
	FRotator BaseRotation = CalculateRotationFromNormal(HitNormal);
	
	// Apply rotation offset (scroll wheel adjustment)
	FRotator NewRotation = BaseRotation;
	NewRotation.Yaw += CurrentRotationOffset;

	// Check if location is valid
	bool bIsValid = CanPlaceAtLocation(HitLocation, HitNormal);

	// Update preview location and rotation
	PreviewLocation = HitLocation;
	PreviewRotation = NewRotation;
	bPreviewLocationValid = bIsValid;

	// Update preview actor
	if (PreviewActor)
	{
		PreviewActor->SetActorLocation(PreviewLocation);
		PreviewActor->SetActorRotation(PreviewRotation);
		PreviewActor->SetActorHiddenInGame(false);

		// Update material based on validity
		if (UStaticMeshComponent* MeshComp = PreviewActor->FindComponentByClass<UStaticMeshComponent>())
		{
			if (bIsValid && PreviewMaterialValid)
			{
				MeshComp->SetMaterial(0, PreviewMaterialValid);
			}
			else if (!bIsValid && PreviewMaterialInvalid)
			{
				MeshComp->SetMaterial(0, PreviewMaterialInvalid);
			}
		}
	}
}

bool UPlacementComponent::PerformGroundTrace(FHitResult& OutHit) const
{
	if (!CameraComponent || !GetWorld())
	{
		return false;
	}

	FVector Start = CameraComponent->GetComponentLocation();
	FVector ForwardVector = CameraComponent->GetForwardVector();
	
	// Use fixed trace distance of 1000
	float TraceDistance = 1000.0f;
	
	FVector End = Start + (ForwardVector * TraceDistance);

	FCollisionQueryParams TraceParams(FName(TEXT("PlacementTrace")), true, GetOwner());
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		TraceParams
	);

	return bHit && OutHit.bBlockingHit;
}

FRotator UPlacementComponent::CalculateRotationFromNormal(const FVector& Normal) const
{
	// Create rotation matrix with Z-axis aligned to surface normal
	FMatrix RotationMatrix = FRotationMatrix::MakeFromZ(Normal);
	return RotationMatrix.Rotator();
}

bool UPlacementComponent::CanPlaceAtLocation(const FVector& Location, const FVector& Normal) const
{
	if (!GetWorld() || !GetOwner())
	{
		return false;
	}

	// Check minimum distance from player
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	float DistanceToPlayer = FVector::Dist(Location, OwnerLocation);
	if (DistanceToPlayer < MinPlacementDistance)
	{
		return false;
	}

	// Check for overlapping actors (especially other placeable items of the same type)
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	if (PreviewActor)
	{
		QueryParams.AddIgnoredActor(PreviewActor);
	}

	bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		ECC_WorldDynamic,
		FCollisionShape::MakeSphere(PlacementCheckRadius),
		QueryParams
	);

	if (bHasOverlap && CurrentPlaceableItem && CurrentPlaceableItem->PlaceableActorClass)
	{
		// Check if any overlapping actor is the same type as what we're placing
		TSubclassOf<AActor> PlaceableClass = CurrentPlaceableItem->PlaceableActorClass;
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			if (Overlap.GetActor() && Overlap.GetActor()->GetClass() == PlaceableClass)
			{
				return false;
			}
		}
	}

	return true;
}

void UPlacementComponent::PlaceItemAtLocation(const FVector& Location, const FRotator& Rotation)
{
	if (!GetWorld() || !CurrentPlaceableItem)
	{
		return;
	}

	// Get the actor class to spawn
	TSubclassOf<AActor> PlaceableClass = CurrentPlaceableItem->PlaceableActorClass;
	if (!PlaceableClass)
	{
		// Default to ASoilPlot for backward compatibility
		PlaceableClass = ASoilPlot::StaticClass();
	}

	// Spawn the placeable actor with the rotation (which already includes the rotation offset)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AActor* NewPlaceable = GetWorld()->SpawnActor<AActor>(PlaceableClass, Location, Rotation, SpawnParams);
	if (NewPlaceable)
	{
		// If it's a soil plot, initialize it with the soil data asset
		if (ASoilPlot* NewSoilPlot = Cast<ASoilPlot>(NewPlaceable))
		{
			if (CurrentPlaceableItem->PlaceableSoilDataAsset)
			{
				NewSoilPlot->Initialize(CurrentPlaceableItem->PlaceableSoilDataAsset);
			}
		}

		// Broadcast delegate
		OnPlaceablePlaced.Broadcast(GetOwner(), NewPlaceable, CurrentPlaceableItem);

		// Consume item from inventory
		if (UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>())
		{
			int32 EquippedSlot = InventoryComp->GetEquippedSlot();
			if (EquippedSlot != INDEX_NONE)
			{
				InventoryComp->ConsumeFromSlot(EquippedSlot, 1);
				
				// Check if we still have the placeable item after consuming
				const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
				if (Slots.IsValidIndex(EquippedSlot))
				{
					const FInventorySlot& Slot = Slots[EquippedSlot];
					if (Slot.IsEmpty() || !Slot.ItemDefinition || !Slot.ItemDefinition->bIsPlaceable)
					{
						// No more placeable items, exit placement mode
						ExitPlacementMode();
					}
				}
			}
		}
	}
}

void UPlacementComponent::UpdatePreviewActor()
{
	if (!GetWorld() || !CurrentPlaceableItem)
	{
		return;
	}

	// Destroy existing preview if any
	DestroyPreviewActor();

	// Get the actor class to use for preview
	TSubclassOf<AActor> PreviewClass = CurrentPlaceableItem->PlaceableActorClass;
	if (!PreviewClass)
	{
		// If PlaceableSoilDataAsset is set, default to ASoilPlot
		if (CurrentPlaceableItem->PlaceableSoilDataAsset)
		{
			PreviewClass = ASoilPlot::StaticClass();
		}
		else if (PreviewActorClass)
		{
			PreviewClass = PreviewActorClass;
		}
	}

	// Spawn preview actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	PreviewActor = GetWorld()->SpawnActor<AActor>(PreviewClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (PreviewActor)
	{
		// If it's a soil plot, initialize with soil data for mesh
		if (ASoilPlot* SoilPlotPreview = Cast<ASoilPlot>(PreviewActor))
		{
			if (CurrentPlaceableItem->PlaceableSoilDataAsset)
			{
				SoilPlotPreview->Initialize(CurrentPlaceableItem->PlaceableSoilDataAsset);
			}
		}

		// Disable collision on preview
		PreviewActor->SetActorEnableCollision(false);

		// Apply default invalid material initially
		if (PreviewMaterialInvalid)
		{
			if (UStaticMeshComponent* MeshComp = PreviewActor->FindComponentByClass<UStaticMeshComponent>())
			{
				MeshComp->SetMaterial(0, PreviewMaterialInvalid);
			}
		}
	}
}

void UPlacementComponent::DestroyPreviewActor()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void UPlacementComponent::ShowPlacementInstructions()
{
	UWorld* World = GetWorld();
	if (!PlacementInstructionWidget && PlacementInstructionWidgetClass && World)
	{
		PlacementInstructionWidget = CreateWidget<UUserWidget>(World, PlacementInstructionWidgetClass);
		if (PlacementInstructionWidget)
		{
			PlacementInstructionWidget->AddToViewport();
		}
	}

	if (PlacementInstructionWidget)
	{
		// Try to cast to InteractionWidget to use its SetPromptText method
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(PlacementInstructionWidget))
		{
			InteractionWidget->SetPromptText(FText::FromString(TEXT("LMB To Place, Mouse Scroll to Rotate")));
			InteractionWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			// If not InteractionWidget, just make it visible
			PlacementInstructionWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UPlacementComponent::HidePlacementInstructions()
{
	if (PlacementInstructionWidget)
	{
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(PlacementInstructionWidget))
		{
			InteractionWidget->HidePrompt();
		}
		else
		{
			PlacementInstructionWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
