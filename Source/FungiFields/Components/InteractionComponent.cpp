#include "InteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "../Interfaces/InteractableInterface.h"
#include "../Widgets/InteractionWidget.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UInteractionComponent::UInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TraceForInteractable();
}

void UInteractionComponent::SetCamera(UCameraComponent* Camera)
{
	CameraComponent = Camera;
}

void UInteractionComponent::Interact(const FInputActionValue& Value)
{
	if (!CameraComponent)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector Start = CameraComponent->GetComponentLocation();
	FVector ForwardVector = CameraComponent->GetForwardVector();
	FVector End = Start + (ForwardVector * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, GetOwner());
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		TraceParams
	);

	DrawDebugLine(World, Start, End, FColor::Green, false, 2.0f);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor->Implements<UInteractableInterface>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Interact with Actor: %s"), *HitActor->GetName());
			IInteractableInterface::Execute_Interact(HitActor, GetOwner());
			ClearInteractable();
		}
	}
}

void UInteractionComponent::TraceForInteractable()
{
	if (!CameraComponent)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params(FName(TEXT("InteractTrace")), true, GetOwner());

	World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
	
	AActor* HitActor = HitResult.GetActor();
	if (HitActor && HitActor->Implements<UInteractableInterface>())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());

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
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Clearing Widget"));
	}
}

void UInteractionComponent::ShowInteractionWidget(AActor* Interactable, const FText& Prompt)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Showing Widget"));
	}

	UWorld* World = GetWorld();
	if (!InteractionWidget && InteractionWidgetClass && World)
	{
		InteractionWidget = CreateWidget<UInteractionWidget>(World, InteractionWidgetClass);
		if (InteractionWidget)
		{
			InteractionWidget->AddToViewport();
		}
	}

	if (InteractionWidget)
	{
		InteractionWidget->SetPromptText(Prompt);
		InteractionWidget->SetVisibility(ESlateVisibility::Visible);
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

