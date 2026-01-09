#include "AChestActor.h"
#include "../Components/ChestInventoryComponent.h"
#include "../Widgets/UChestWidget.h"
#include "../Characters/FungiFieldsCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AChestActor::AChestActor()
{
	ChestInventoryComponent = CreateDefaultSubobject<UChestInventoryComponent>(TEXT("ChestInventoryComponent"));
}

void AChestActor::BeginPlay()
{
	Super::BeginPlay();
}

void AChestActor::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	AFungiFieldsCharacter* PlayerCharacter = Cast<AFungiFieldsCharacter>(Interactor);
	if (!PlayerCharacter)
	{
		return;
	}

	if (ChestWidgetInstance && ChestWidgetInstance->IsInViewport())
	{
		CloseChestWidget();
	}
	else
	{
		OpenChestWidgetForPlayer(Interactor);
	}

	Super::Interact_Implementation(Interactor);
}

FText AChestActor::GetInteractionText_Implementation()
{
	return FText::FromString("Press E to Open Chest");
}

void AChestActor::OpenChestWidgetForPlayer(AActor* Interactor)
{
	if (!ChestWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChestWidgetClass not set on %s"), *GetName());
		return;
	}

	AFungiFieldsCharacter* PlayerCharacter = Cast<AFungiFieldsCharacter>(Interactor);
	if (!PlayerCharacter)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!ChestWidgetInstance)
	{
		ChestWidgetInstance = CreateWidget<UChestWidget>(World, ChestWidgetClass);
	}

	if (ChestWidgetInstance)
	{
		ChestWidgetInstance->SetupInventories(PlayerCharacter->InventoryComponent, ChestInventoryComponent);
		ChestWidgetInstance->OnChestWidgetClosed.AddDynamic(this, &AChestActor::OnChestWidgetClosed);
		ChestWidgetInstance->AddToViewport();

		if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ChestWidgetInstance->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
	}
}

void AChestActor::OnChestWidgetClosed()
{
	ChestWidgetInstance = nullptr;
	
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
		}
	}

	OnChestClosed.Broadcast();
}

void AChestActor::CloseChestWidget()
{
	if (ChestWidgetInstance)
	{
		ChestWidgetInstance->CloseWidget();
	}
}



