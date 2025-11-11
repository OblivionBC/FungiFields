#include "InteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "../Widgets/InteractionWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

AInteractableActor::AInteractableActor()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(RootComponent);

    Proximity = CreateDefaultSubobject<USphereComponent>("Proximity");
    Proximity->SetupAttachment(RootComponent);
    Proximity->InitSphereRadius(150.f);
    Proximity->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Proximity->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Proximity->OnComponentBeginOverlap.AddDynamic(this, &AInteractableActor::OnProximityEnter);
    Proximity->OnComponentEndOverlap.AddDynamic(this, &AInteractableActor::OnProximityExit);
    Proximity->SetupAttachment(RootComponent);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
    Mesh->SetupAttachment(Proximity);
    
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

void AInteractableActor::OnProximityEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                          bool bFromSweep, const FHitResult& SweepResult)
{
    if (Cast<APawn>(OtherActor))
    {
        if (UInteractionWidget* W = GetInteractionWidget())
        {
            W->SetPromptText(GetInteractionText_Implementation());
            W->ShowPrompt();
        }

        Widget->SetVisibility(true);
    }
}

void AInteractableActor::OnProximityExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (Cast<APawn>(OtherActor))
    {
        if (UInteractionWidget* W = GetInteractionWidget())
        {
            W->HidePrompt();
        }

        Widget->SetVisibility(false);
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

void AInteractableActor::Interact_Implementation(AActor* Interactor)
{
    // Example: Print interaction feedback
    UE_LOG(LogTemp, Log, TEXT("Interacted with by: %s"), *GetNameSafe(Interactor));
    if(GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Interact Received "));
}
