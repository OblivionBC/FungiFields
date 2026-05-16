#include "UBT_DialogueComponent.h"
#include "../Widgets/UBT_DialogueWidget.h"
#include "UShopComponent.h"
#include "UQuestOfferComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UBT_DialogueComponent::UBT_DialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBT_DialogueComponent::InitiateDialogue(AActor* Interactor)
{
	if (!Interactor || !DialogueWidgetClass)
		return;

	APawn* InteractorPawn = Cast<APawn>(Interactor);
	APlayerController* PC = InteractorPawn ? Cast<APlayerController>(InteractorPawn->GetController()) : nullptr;
	if (!PC)
		return;

	CachedPC = PC;

	StopOwnerMovement();
	FaceInteractor(Interactor);

	ActiveWidget = CreateWidget<UBT_DialogueWidget>(PC, DialogueWidgetClass);
	if (!ActiveWidget)
		return;

	ActiveWidget->SetDialogueComponent(this);
	ActiveWidget->AddToViewport();
	ApplyInputMode(true);
}

void UBT_DialogueComponent::CloseDialogue()
{
	if (!ActiveWidget)
		return;

	UBT_DialogueWidget* WidgetToClose = ActiveWidget;
	ActiveWidget = nullptr;

	WidgetToClose->RemoveFromParent();
	ApplyInputMode(false);
	RestoreOwnerMovement();
	CachedPC.Reset();
}

FText UBT_DialogueComponent::GetNextTalkLine()
{
	if (TalkLines.IsEmpty())
	{
		return FText::Format(
			NSLOCTEXT("BTDialogue", "DefaultGreeting", "Hello! I'm {0}."),
			NpcName);
	}

	const FText Line = TalkLines[TalkLineIndex];
	TalkLineIndex = (TalkLineIndex + 1) % TalkLines.Num();
	return Line;
}

APlayerController* UBT_DialogueComponent::GetCachedController() const
{
	return CachedPC.Get();
}

UShopComponent* UBT_DialogueComponent::GetOwnerShopComponent() const
{
	AActor* Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UShopComponent>() : nullptr;
}

UQuestOfferComponent* UBT_DialogueComponent::GetOwnerQuestOfferComponent() const
{
	AActor* Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UQuestOfferComponent>() : nullptr;
}

void UBT_DialogueComponent::FaceInteractor(const AActor* Interactor) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !Interactor) return;

	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(
		Owner->GetActorLocation(), Interactor->GetActorLocation());

	const FRotator Current = Owner->GetActorRotation();
	Owner->SetActorRotation(FRotator(Current.Pitch, LookAt.Yaw, Current.Roll));
}

void UBT_DialogueComponent::StopOwnerMovement()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	if (AAIController* AIC = Cast<AAIController>(OwnerChar->GetController()))
		AIC->StopMovement();

	if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
		MoveComp->DisableMovement();
}

void UBT_DialogueComponent::RestoreOwnerMovement()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
		MoveComp->SetMovementMode(MOVE_Walking);
}

void UBT_DialogueComponent::ApplyInputMode(bool bOpen)
{
	APlayerController* PC = CachedPC.Get();
	if (!PC) return;

	if (bOpen)
	{
		FInputModeGameAndUI InputMode;
		if (ActiveWidget)
			InputMode.SetWidgetToFocus(ActiveWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

