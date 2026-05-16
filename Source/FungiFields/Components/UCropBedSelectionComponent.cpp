#include "UCropBedSelectionComponent.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "../Components/UVillagerNeedsComponent.h"
#include "../Actors/ASoilPlot.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UCropBedSelectionComponent::UCropBedSelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCropBedSelectionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCropBedSelectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController()))
		{
			if (PC->WasInputKeyJustPressed(EKeys::Escape))
			{
				EndSelection();
				return;
			}
		}
	}

	ASoilPlot* NewHovered = TraceForPlot();
	if (NewHovered != HoveredPlot.Get())
	{
		ASoilPlot* OldHovered = HoveredPlot.Get();
		HoveredPlot = NewHovered;

		// Refresh highlights on the two changed plots only — no need to iterate everything.
		if (IsValid(OldHovered))
		{
			AFarmerVillagerCharacter* Villager = TargetVillager.Get();
			OldHovered->SetSelectionHighlight(false, Villager && Villager->IsPlotAssigned(OldHovered));
		}
		if (IsValid(NewHovered))
		{
			AFarmerVillagerCharacter* Villager = TargetVillager.Get();
			NewHovered->SetSelectionHighlight(true, Villager && Villager->IsPlotAssigned(NewHovered));
		}

		OnHoveredPlotChanged.Broadcast(NewHovered);
	}
}

void UCropBedSelectionComponent::StartSelection(AFarmerVillagerCharacter* Villager)
{
	if (!IsValid(Villager)) return;

	TargetVillager = Villager;
	Villager->OnAssignedPlotsChanged.AddDynamic(this, &UCropBedSelectionComponent::HandleVillagerPlotsChanged);

	PrimaryComponentTick.SetTickFunctionEnable(true);

	if (SelectionOverlayWidgetClass)
	{
		APlayerController* PC = nullptr;
		if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
			PC = Cast<APlayerController>(OwnerChar->GetController());

		if (PC && !OverlayWidget)
			OverlayWidget = CreateWidget<UUserWidget>(PC, SelectionOverlayWidgetClass);

		if (OverlayWidget)
			OverlayWidget->AddToViewport(9999);
	}

	RefreshAllHighlights();
	OnSelectionModeChanged.Broadcast(true);
	OnBedCountChanged.Broadcast(GetCurrentAssignedBeds(), GetMaxBeds());
}

void UCropBedSelectionComponent::EndSelection()
{
	ClearAllHighlights();

	if (AFarmerVillagerCharacter* Villager = TargetVillager.Get())
		Villager->OnAssignedPlotsChanged.RemoveDynamic(this, &UCropBedSelectionComponent::HandleVillagerPlotsChanged);

	TargetVillager = nullptr;
	HoveredPlot = nullptr;
	PrimaryComponentTick.SetTickFunctionEnable(false);

	if (OverlayWidget)
	{
		OverlayWidget->RemoveFromParent();
		OverlayWidget = nullptr;
	}

	OnSelectionModeChanged.Broadcast(false);
}

void UCropBedSelectionComponent::HandleInteract()
{
	AFarmerVillagerCharacter* Villager = TargetVillager.Get();
	if (!Villager) return;

	ASoilPlot* Plot = TraceForPlot();
	if (!Plot) return;

	if (Villager->IsPlotAssigned(Plot))
		Villager->UnassignPlot(Plot);
	else
		Villager->AssignPlot(Plot);
}

void UCropBedSelectionComponent::HandleVillagerPlotsChanged()
{
	RefreshAllHighlights();
	OnBedCountChanged.Broadcast(GetCurrentAssignedBeds(), GetMaxBeds());
}

void UCropBedSelectionComponent::RefreshAllHighlights()
{
	AFarmerVillagerCharacter* Villager = TargetVillager.Get();
	if (!Villager) return;

	ASoilPlot* Hovered = HoveredPlot.Get();

	for (TWeakObjectPtr<ASoilPlot>& WeakPlot : Villager->GetAssignedPlotsRaw())
	{
		if (ASoilPlot* Plot = WeakPlot.Get())
			Plot->SetSelectionHighlight(Plot == Hovered, true);
	}

	if (IsValid(Hovered) && !Villager->IsPlotAssigned(Hovered))
		Hovered->SetSelectionHighlight(true, false);
}

void UCropBedSelectionComponent::ClearAllHighlights()
{
	AFarmerVillagerCharacter* Villager = TargetVillager.Get();
	if (Villager)
	{
		for (TWeakObjectPtr<ASoilPlot>& WeakPlot : Villager->GetAssignedPlotsRaw())
		{
			if (ASoilPlot* Plot = WeakPlot.Get())
				Plot->ClearSelectionHighlight();
		}
	}

	if (ASoilPlot* Hovered = HoveredPlot.Get())
		Hovered->ClearSelectionHighlight();
}

int32 UCropBedSelectionComponent::GetCurrentAssignedBeds() const
{
	AFarmerVillagerCharacter* Villager = TargetVillager.Get();
	return Villager ? Villager->GetAssignedPlotCount() : 0;
}

int32 UCropBedSelectionComponent::GetMaxBeds() const
{
	AFarmerVillagerCharacter* Villager = TargetVillager.Get();
	if (!Villager) return 0;
	UVillagerNeedsComponent* Needs = Villager->GetNeedsComponent();
	return Needs ? Needs->GetMaxAssignedCropBeds() : 0;
}

FText UCropBedSelectionComponent::GetTargetVillagerName() const
{
	AFarmerVillagerCharacter* Villager = TargetVillager.Get();
	return Villager ? Villager->GetVillagerDisplayName() : FText::GetEmpty();
}

ASoilPlot* UCropBedSelectionComponent::TraceForPlot() const
{
	UCameraComponent* Camera = GetCamera();
	if (!Camera) return nullptr;

	const FVector Start = Camera->GetComponentLocation();
	const FVector End   = Start + Camera->GetForwardVector() * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(FName(TEXT("BedSelectTrace")), false, GetOwner());
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		return Cast<ASoilPlot>(Hit.GetActor());

	return nullptr;
}

UCameraComponent* UCropBedSelectionComponent::GetCamera() const
{
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
		return OwnerChar->FindComponentByClass<UCameraComponent>();
	return nullptr;
}
