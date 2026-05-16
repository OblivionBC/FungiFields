#include "InteractionComponent.h"
#include "UCropBedSelectionComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "../Interfaces/IInteractableInterface.h"
#include "../Widgets/UInteractionWidget.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

namespace
{
	constexpr int32 InteractionPromptZOrder = 1;
}

UInteractionComponent::UInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TracePollingTimer,
			this,
			&UInteractionComponent::TraceForInteractable,
			0.0667f,
			true
		);
	}
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TracePollingTimer);
		World->GetTimerManager().ClearTimer(InteractableResetTimer);
	}
	ClearInteractable();
	Super::EndPlay(EndPlayReason);
}

void UInteractionComponent::SetCamera(UCameraComponent* Camera)
{
	CameraComponent = Camera;
}

void UInteractionComponent::Interact(const FInputActionValue& Value)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!CameraComponent || !World || !Owner)
		return;

	if (UCropBedSelectionComponent* SelectionComp = Owner->FindComponentByClass<UCropBedSelectionComponent>())
	{
		if (SelectionComp->IsSelectionActive())
		{
			SelectionComp->HandleInteract();
			return;
		}
	}

	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), false, Owner);
	TraceParams.bReturnPhysicalMaterial = false;

	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(TraceRadius);
	if (World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, SweepShape, TraceParams))
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (HitActor->Implements<UInteractableInterface>())
			{
				if (AFarmerVillagerCharacter* Villager = Cast<AFarmerVillagerCharacter>(HitActor))
				{
					CurrentInteractedVillager = Villager;
					OnVillagerInteractionStarted.Broadcast(Villager);
				}
				IInteractableInterface::Execute_Interact(HitActor, Owner);
				ClearInteractable();
			}
		}
	}
}

void UInteractionComponent::TraceForInteractable()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!CameraComponent || !World || !Owner)
		return;

	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params(FName(TEXT("InteractTrace")), false, Owner);

	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(TraceRadius);
	World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, SweepShape, Params);
	
	AActor* HitActor = HitResult.GetActor();
	if (HitActor && HitActor->Implements<UInteractableInterface>())
	{
		if (HitActor != LastInteractable)
		{
			FText Prompt = IInteractableInterface::Execute_GetInteractionText(HitActor);
			ShowInteractionWidget(HitActor, Prompt);
			LastInteractable = HitActor;
			World->GetTimerManager().ClearTimer(InteractableResetTimer);
		}
	}
	else
	{
		HideInteractionWidget();
		if (LastInteractable && !IsValid(LastInteractable))
		{
			ClearInteractable();
		}
		else
		{
			World->GetTimerManager().SetTimer(
				InteractableResetTimer,
				this,
				&UInteractionComponent::ClearInteractable,
				ClearDelay,
				false
			);
		}
	}
}

void UInteractionComponent::ClearInteractable()
{
	LastInteractable = nullptr;
	HideInteractionWidget();
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InteractableResetTimer);
	}
}

void UInteractionComponent::ShowInteractionWidget(AActor* Interactable, const FText& Prompt)
{
	if (!InteractionWidget && InteractionWidgetClass)
	{
		APawn* PawnOwner = Cast<APawn>(GetOwner());
		APlayerController* PC = PawnOwner ? Cast<APlayerController>(PawnOwner->GetController()) : nullptr;
		if (PC)
		{
			InteractionWidget = CreateWidget<UInteractionWidget>(PC, InteractionWidgetClass);
			if (InteractionWidget)
				InteractionWidget->AddToViewport(InteractionPromptZOrder);
		}
	}

	if (InteractionWidget)
	{
		InteractionWidget->SetPromptText(Prompt);
		InteractionWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	LastInteractable = Interactable;
}

void UInteractionComponent::HideInteractionWidget()
{
	if (InteractionWidget)
	{
		InteractionWidget->HidePrompt();
	}
	LastInteractable = nullptr;
}

