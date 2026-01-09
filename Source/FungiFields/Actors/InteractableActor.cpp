#include "InteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "../Widgets/InteractionWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

AInteractableActor::AInteractableActor()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(RootComponent);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
    Mesh->SetupAttachment(RootComponent);
    
    Widget = CreateDefaultSubobject<UWidgetComponent>("Widget");
    Widget->SetupAttachment(Mesh);
    Widget->SetWidgetSpace(EWidgetSpace::World);
    Widget->SetDrawAtDesiredSize(true);
    Widget->SetPivot(FVector2D(0.5f, 0.5f));
    Widget->SetTwoSided(true);
    Widget->SetVisibility(false);
}

void AInteractableActor::BeginPlay()
{
    Super::BeginPlay();

    if (WidgetClass)
    {
        Widget->SetWidgetClass(WidgetClass);
    }

    UpdateWidgetTransform();
}

void AInteractableActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateWidgetTransform();

    if (bFacePlayerCamera)
    {
        BillboardToCamera();
    }
}

void AInteractableActor::UpdateWidgetTransform() const
{
    const FVector BaseLoc = GetActorLocation();
    Widget->SetWorldLocation(BaseLoc + FVector(0.f, 0.f, WidgetHeightOffset));
}

void AInteractableActor::BillboardToCamera()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    APlayerCameraManager* PCM = PC->PlayerCameraManager;
    if (!PCM) return;

    const FVector CamLoc = PCM->GetCameraLocation();
    const FVector WidgetLoc = Widget->GetComponentLocation();
    const FRotator LookAtRot = (CamLoc - WidgetLoc).Rotation();
    Widget->SetWorldRotation(FRotator(0.f, LookAtRot.Yaw + 180.f, 0.f));
}

UInteractionWidget* AInteractableActor::GetInteractionWidget() const
{
    return Cast<UInteractionWidget>(Widget->GetUserWidgetObject());
}

FText AInteractableActor::GetInteractionText_Implementation()
{
    return FText::FromString("Press E to Interact");
}

FText AInteractableActor::GetTooltipText_Implementation() const
{
    return FText::FromString("Press E to Interact");
}

void AInteractableActor::Interact_Implementation(AActor* Interactor)
{
    UE_LOG(LogTemp, Log, TEXT("Interacted with by: %s"), *GetNameSafe(Interactor));
    if(GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Interact Received "));
}
