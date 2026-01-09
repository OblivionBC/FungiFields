#include "UPlacementComponent.h"
#include "Camera/CameraComponent.h"
#include "../Data/UItemDataAsset.h"
#include "../Data/USoilDataAsset.h"
#include "../Data/USoilContainerDataAsset.h"
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
#include "Components/SceneComponent.h"

UPlacementComponent::UPlacementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	bIsInPlacementMode = false;
	CurrentPlaceableItem = nullptr;
	PreviewActor = nullptr;
	PreviewLocation = FVector::ZeroVector;
	TargetBottomLocation = FVector::ZeroVector;
	PreviewRotation = FRotator::ZeroRotator;
	bPreviewLocationValid = false;
	CachedBottomOffset = 0.0f;
	bBottomOffsetCached = false;
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

	if (bIsInPlacementMode && CurrentPlaceableItem == PlaceableItem)
	{
		return;
	}

	CurrentPlaceableItem = PlaceableItem;
	bIsInPlacementMode = true;
	CurrentRotationOffset = 0.0f;
	
	UpdatePreviewActor();
	UpdatePreview();
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

	float ScrollDelta = Value.Get<float>();
	CurrentRotationOffset += (ScrollDelta * RotationAdjustmentStep);
	
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
			USoilContainerDataAsset* ContainerData = HitSoilPlot->GetContainerDataAsset();
			
			if (!ContainerData)
			{
				USoilDataAsset* SoilData = HitSoilPlot->GetSoilDataAsset();
				if (!SoilData && HitSoilPlot->GetSoilComponent())
				{
					SoilData = HitSoilPlot->GetSoilComponent()->GetSoilData();
				}

				if (SoilData)
				{
					OnPlaceablePickedUp.Broadcast(GetOwner(), SoilData);
					HitSoilPlot->Destroy();
				}
			}
			else
			{
				OnContainerPickedUp.Broadcast(GetOwner(), ContainerData);
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

	FVector HitLocation = HitResult.Location;
	FVector HitNormal = HitResult.Normal;
	
	float OffsetZ = CurrentPlaceableItem->PreviewOffsetZ;
	FVector TargetBottom = HitLocation;
	TargetBottom.Z += OffsetZ;
	
	if (PreviewActor && !bBottomOffsetCached)
	{
		CachedBottomOffset = CalculateActorBottomOffset(PreviewActor);
		bBottomOffsetCached = true;
	}
	
	FVector ActorRootLocation = TargetBottom;
	ActorRootLocation.Z -= CachedBottomOffset;

	FRotator BaseRotation = CalculateRotationFromNormal(HitNormal);
	FRotator NewRotation = BaseRotation;
	NewRotation.Yaw += CurrentRotationOffset;

	bool bIsValid = CanPlaceAtLocation(ActorRootLocation, HitNormal);

	PreviewLocation = ActorRootLocation;
	TargetBottomLocation = TargetBottom;
	PreviewRotation = NewRotation;
	bPreviewLocationValid = bIsValid;

	if (PreviewActor)
	{
		PreviewActor->SetActorLocation(PreviewLocation);
		PreviewActor->SetActorRotation(PreviewRotation);
		PreviewActor->SetActorHiddenInGame(false);

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
	FMatrix RotationMatrix = FRotationMatrix::MakeFromZ(Normal);
	return RotationMatrix.Rotator();
}

bool UPlacementComponent::CanPlaceAtLocation(const FVector& Location, const FVector& Normal) const
{
	if (!GetWorld() || !GetOwner())
	{
		return false;
	}

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	float DistanceToPlayer = FVector::Dist(Location, OwnerLocation);
	if (DistanceToPlayer < MinPlacementDistance)
	{
		return false;
	}

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

	TSubclassOf<AActor> PlaceableClass = CurrentPlaceableItem->PlaceableActorClass;
	if (!PlaceableClass)
	{
		PlaceableClass = ASoilPlot::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AActor* NewPlaceable = GetWorld()->SpawnActor<AActor>(PlaceableClass, Location, Rotation, SpawnParams);
	if (NewPlaceable)
	{
		if (ASoilPlot* NewSoilPlot = Cast<ASoilPlot>(NewPlaceable))
		{
			USoilContainerDataAsset* ContainerData = CurrentPlaceableItem->PlaceableContainerDataAsset;
			if (!ContainerData && CurrentPlaceableItem->PlaceableSoilDataAsset)
			{
				NewSoilPlot->Initialize(CurrentPlaceableItem->PlaceableSoilDataAsset);
			}
			else
			{
				NewSoilPlot->Initialize(ContainerData, nullptr);
			}
		}

		float BottomOffset = CachedBottomOffset;
		if (!bBottomOffsetCached)
		{
			NewPlaceable->RegisterAllComponents();
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			NewPlaceable->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
			for (UPrimitiveComponent* Comp : PrimitiveComponents)
			{
				if (Comp)
				{
					Comp->UpdateBounds();
				}
			}
			BottomOffset = CalculateActorBottomOffset(NewPlaceable);
		}

		FVector AdjustedLocation = TargetBottomLocation;
		AdjustedLocation.Z -= BottomOffset;
		NewPlaceable->SetActorLocation(AdjustedLocation);

		OnPlaceablePlaced.Broadcast(GetOwner(), NewPlaceable, CurrentPlaceableItem);

		if (UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>())
		{
			int32 EquippedSlot = InventoryComp->GetEquippedSlot();
			if (EquippedSlot != INDEX_NONE)
			{
				InventoryComp->ConsumeFromSlot(EquippedSlot, 1);
				
				const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
				if (Slots.IsValidIndex(EquippedSlot))
				{
					const FInventorySlot& Slot = Slots[EquippedSlot];
					if (Slot.IsEmpty() || !Slot.ItemDefinition || !Slot.ItemDefinition->bIsPlaceable)
					{
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

	DestroyPreviewActor();
	
	CachedBottomOffset = 0.0f;
	bBottomOffsetCached = false;

	TSubclassOf<AActor> PreviewClass = CurrentPlaceableItem->PlaceableActorClass;
	if (!PreviewClass)
	{
		if (CurrentPlaceableItem->PlaceableContainerDataAsset || CurrentPlaceableItem->PlaceableSoilDataAsset)
		{
			PreviewClass = ASoilPlot::StaticClass();
		}
		else if (PreviewActorClass)
		{
			PreviewClass = PreviewActorClass;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	PreviewActor = GetWorld()->SpawnActor<AActor>(PreviewClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (PreviewActor)
	{
		if (ASoilPlot* SoilPlotPreview = Cast<ASoilPlot>(PreviewActor))
		{
			USoilContainerDataAsset* ContainerData = CurrentPlaceableItem->PlaceableContainerDataAsset;
			if (!ContainerData && CurrentPlaceableItem->PlaceableSoilDataAsset)
			{
				SoilPlotPreview->Initialize(CurrentPlaceableItem->PlaceableSoilDataAsset);
			}
			else
			{
				SoilPlotPreview->Initialize(ContainerData, nullptr);
			}
		}

		PreviewActor->SetActorEnableCollision(false);
		
		PreviewActor->RegisterAllComponents();
		
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		PreviewActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* Comp : PrimitiveComponents)
		{
			if (Comp)
			{
				Comp->UpdateBounds();
			}
		}
		
		CachedBottomOffset = CalculateActorBottomOffset(PreviewActor);
		bBottomOffsetCached = true;

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
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(PlacementInstructionWidget))
		{
			InteractionWidget->SetPromptText(FText::FromString(TEXT("LMB To Place, Mouse Scroll to Rotate")));
			InteractionWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
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

float UPlacementComponent::CalculateActorBottomOffset(AActor* Actor) const
{
	if (!Actor)
	{
		return 0.0f;
	}

	FTransform ActorTransform = Actor->GetActorTransform();

	TArray<UStaticMeshComponent*> StaticMeshComponents;
	Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	
	UStaticMeshComponent* MainMesh = nullptr;
	for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
	{
		if (MeshComp && MeshComp->GetStaticMesh() != nullptr)
		{
			FString ComponentName = MeshComp->GetName();
			if (!ComponentName.Contains(TEXT("Widget"), ESearchCase::IgnoreCase))
			{
				MainMesh = MeshComp;
				break;
			}
		}
	}
	
	if (MainMesh && MainMesh->GetStaticMesh())
	{
		FBoxSphereBounds MeshBounds = MainMesh->GetStaticMesh()->GetBounds();
		FBox MeshLocalBox = MeshBounds.GetBox();
		
		if (MeshLocalBox.IsValid)
		{
			FTransform MeshTransform = MainMesh->GetComponentTransform();
			FTransform RelativeTransform = MeshTransform.GetRelativeTransform(ActorTransform);
			
			FBox ActorSpaceBox = MeshLocalBox.TransformBy(RelativeTransform);
			
			if (ActorSpaceBox.IsValid)
			{
				return ActorSpaceBox.Min.Z;
			}
		}
	}

	FVector Origin, BoxExtent;
	Actor->GetActorBounds(false, Origin, BoxExtent);
	
	if (BoxExtent.SizeSquared() > SMALL_NUMBER)
	{
		FVector WorldBottom = Origin - FVector(0.0f, 0.0f, BoxExtent.Z);
		FVector LocalBottom = ActorTransform.InverseTransformPosition(WorldBottom);
		return LocalBottom.Z;
	}

	FBox CombinedBounds(ForceInit);
	bool bHasBounds = false;

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* Comp : PrimitiveComponents)
	{
		if (Comp)
		{
			FString ComponentName = Comp->GetName();
			if (ComponentName.Contains(TEXT("Widget"), ESearchCase::IgnoreCase))
			{
				continue;
			}
			
			FBoxSphereBounds ComponentBounds;
			
			if (Comp->IsRegistered() && Comp->Bounds.SphereRadius > 0.0f)
			{
				ComponentBounds = Comp->Bounds;
			}
			else
			{
				ComponentBounds = Comp->CalcLocalBounds();
			}
			
			FBox LocalBox = ComponentBounds.GetBox();
			
			if (LocalBox.IsValid)
			{
				FTransform ComponentTransform = Comp->GetComponentTransform();
				FTransform RelativeTransform = ComponentTransform.GetRelativeTransform(ActorTransform);
				
				FBox ActorSpaceBox = LocalBox.TransformBy(RelativeTransform);
				
				if (ActorSpaceBox.IsValid)
				{
					if (!bHasBounds)
					{
						CombinedBounds = ActorSpaceBox;
						bHasBounds = true;
					}
					else
					{
						CombinedBounds += ActorSpaceBox;
					}
				}
			}
		}
	}

	if (bHasBounds && CombinedBounds.IsValid)
	{
		return CombinedBounds.Min.Z;
	}

	return 0.0f;
}
