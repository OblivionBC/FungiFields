#include "AChestActor.h"
#include "../Components/ChestInventoryComponent.h"
#include "../Widgets/UChestWidget.h"
#include "../Characters/FungiFieldsCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AChestActor::AChestActor()
{
	// Use the specialized chest inventory component that has replication enabled by default
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

	// Only allow player characters to interact
	AFungiFieldsCharacter* PlayerCharacter = Cast<AFungiFieldsCharacter>(Interactor);
	if (!PlayerCharacter)
	{
		return;
	}

	// Open or close the chest widget
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

	// Create or get the chest widget
	if (!ChestWidgetInstance)
	{
		ChestWidgetInstance = CreateWidget<UChestWidget>(World, ChestWidgetClass);
	}

	if (ChestWidgetInstance)
	{
		// Initialize widget with player and chest inventories
		ChestWidgetInstance->SetupInventories(PlayerCharacter->InventoryComponent, ChestInventoryComponent);
		
		// Bind to close delegate
		ChestWidgetInstance->OnChestWidgetClosed.AddDynamic(this, &AChestActor::OnChestWidgetClosed);
		
		ChestWidgetInstance->AddToViewport();

		// Set input mode to UI
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
	// Widget has been closed, clean up
	ChestWidgetInstance = nullptr;
	
	// Restore game input mode
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
		}
	}

	// Broadcast chest closed event
	OnChestClosed.Broadcast();
}

void AChestActor::CloseChestWidget()
{
	if (ChestWidgetInstance)
	{
		// Close via widget method (will broadcast delegate)
		ChestWidgetInstance->CloseWidget();
	}
}

