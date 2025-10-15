// Copyright Epic Games, Inc. All Rights Reserved.

#include "FungiFieldsCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "SWarningOrErrorBox.h"
#include "Blueprint/UserWidget.h"
#include "../Interfaces/InteractableInterface.h"
#include "Misc/CoreMiscDefines.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AFungiFieldsCharacter

void AFungiFieldsCharacter::Interact_Implementation(AActor* Interactor)
{
	IInteractableInterface::Interact_Implementation(Interactor);
	UE_LOG(LogTemp, Log, TEXT("Implementation called!"));
}

AFungiFieldsCharacter::AFungiFieldsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AFungiFieldsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TraceForInteractable();
}


void AFungiFieldsCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFungiFieldsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFungiFieldsCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFungiFieldsCharacter::Look);

		// Game Mechanics
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AFungiFieldsCharacter::Interact);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AFungiFieldsCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}


void AFungiFieldsCharacter::Interact(const FInputActionValue& Value)
{
	FVector Start = FollowCamera->GetComponentLocation(); // Or GetActorLocation()
	FVector ForwardVector = FollowCamera->GetForwardVector(); // Use camera for better precision
	FVector End = Start + (ForwardVector * 575.0f); // Trace distance

	FHitResult HitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, this);
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		TraceParams
	);

	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.0f);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor->Implements<UInteractableInterface>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Interact with Actor: %s"), *HitActor->GetName());
			IInteractableInterface::Execute_Interact(HitActor, this);
		}
	}
}


void AFungiFieldsCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFungiFieldsCharacter::TraceForInteractable()
{
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + (FollowCamera->GetForwardVector() * 575.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params(FName(TEXT("InteractTrace")), true, this);

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
	
	AActor* HitActor = HitResult.GetActor();
	if (HitActor && HitActor->Implements<UInteractableInterface>())
	{
		if(GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Hiy Actor "));	
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());

		if (HitActor != LastInteractable)
		{
			FText Prompt = IInteractableInterface::Execute_GetInteractionText(HitActor);
			ShowInteractionWidget(HitActor, Prompt);
			LastInteractable = HitActor;
			GetWorld()->GetTimerManager().ClearTimer(InteractableResetTimer);
		}
	}
	else
	{
		// Start/reset the timer to clear the widget
		GetWorld()->GetTimerManager().SetTimer(
			InteractableResetTimer,
			this,
			&AFungiFieldsCharacter::ClearInteractable,
			3.0f,
			false
		);
	}
}

void AFungiFieldsCharacter::ClearInteractable()
{
	LastInteractable = nullptr;
	HideInteractionWidget();
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Clearing Widget"));
}

void AFungiFieldsCharacter::ShowInteractionWidget(AActor* Interactable, const FText& Prompt)
{
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Showing Widget "));
	if (!InteractionWidget && InteractionWidgetClass)
	{
		InteractionWidget = CreateWidget<UInteractionWidget>(GetWorld(), InteractionWidgetClass);
		InteractionWidget->AddToViewport();
	}

	if (InteractionWidget)
	{
		// Assuming your widget has a method to set the prompt text
		InteractionWidget->SetPromptText(Prompt);

		InteractionWidget->SetVisibility(ESlateVisibility::Visible);
	}
	LastInteractable = Interactable;
}

void AFungiFieldsCharacter::HideInteractionWidget()
{
	if (InteractionWidget)
	{
		InteractionWidget->HidePrompt();
	}
}


