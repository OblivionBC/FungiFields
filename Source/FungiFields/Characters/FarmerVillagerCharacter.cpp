#include "FarmerVillagerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "../Components/UFarmingComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Components/AI/FarmerTargetingComponent.h"
#include "../AI/FarmerAIController.h"
#include "../Data/UToolDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "../Data/UCropDataAsset.h"
#include "../Inventory/FInventorySlot.h"
#include "../Widgets/UVillagerManagementWidget.h"
#include "../Widgets/UVillagerDialogueWidget.h"
#include "../Subsystems/UMilestoneSubsystem.h"

AFarmerVillagerCharacter::AFarmerVillagerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	FarmingComponent = CreateDefaultSubobject<UFarmingComponent>(TEXT("FarmingComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	TargetingComponent = CreateDefaultSubobject<UFarmerTargetingComponent>(TEXT("TargetingComponent"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AFarmerAIController::StaticClass();

	GetCharacterMovement()->MaxWalkSpeed = 280.0f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void AFarmerVillagerCharacter::BeginPlay()
{
	Super::BeginPlay();

	HomeLocation = GetActorLocation();

	if (UWorld* World = GetWorld())
	{
		if (UMilestoneSubsystem* MilestoneSubsystem = World->GetSubsystem<UMilestoneSubsystem>())
		{
			MilestoneSubsystem->ReportEvent(TEXT("VillagersRecruited"));
		}
	}

	if (FarmingComponent)
	{
		FarmingComponent->OnCropHarvested.AddDynamic(this, &AFarmerVillagerCharacter::OnCropHarvestedForMilestone);
		FarmingComponent->OnSeedPlanted.AddDynamic(this, &AFarmerVillagerCharacter::OnSeedPlantedForMilestone);
	}
}

void AFarmerVillagerCharacter::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	APawn* InteractorPawn = Cast<APawn>(Interactor);
	APlayerController* PC = InteractorPawn ? Cast<APlayerController>(InteractorPawn->GetController()) : nullptr;
	if (!PC)
	{
		return;
	}

	if (VillagerDialogueWidgetClass)
	{
		UVillagerDialogueWidget* DialogueWidget = CreateWidget<UVillagerDialogueWidget>(PC, VillagerDialogueWidgetClass);
		if (DialogueWidget)
		{
			LastInteractorPC = PC;
			DialogueWidget->SetVillager(this);
			DialogueWidget->OnManageRequested.AddDynamic(this, &AFarmerVillagerCharacter::OnDialogueManageRequested);
			DialogueWidget->OnDialogueClosed.AddDynamic(this, &AFarmerVillagerCharacter::OnDialogueClosed);
			DialogueWidget->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(DialogueWidget->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
			return;
		}
	}

	OpenManagementWidget(PC);
}

void AFarmerVillagerCharacter::OnDialogueManageRequested()
{
	if (LastInteractorPC.IsValid())
	{
		OpenManagementWidget(LastInteractorPC.Get());
	}
}

void AFarmerVillagerCharacter::OnDialogueClosed()
{
	if (APlayerController* PC = LastInteractorPC.Get())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

void AFarmerVillagerCharacter::OpenManagementWidget(APlayerController* PC)
{
	if (!PC || !VillagerManagementWidgetClass)
	{
		return;
	}

	UVillagerManagementWidget* Widget = CreateWidget<UVillagerManagementWidget>(PC, VillagerManagementWidgetClass);
	if (!Widget)
	{
		return;
	}

	Widget->SetVillager(this);
	Widget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}

FText AFarmerVillagerCharacter::GetInteractionText_Implementation()
{
	return FText::Format(INVTEXT("Talk to {0}"), VillagerDisplayName);
}

FText AFarmerVillagerCharacter::GetTooltipText_Implementation() const
{
	return VillagerDisplayName;
}

void AFarmerVillagerCharacter::SetAssignedRole(EFarmerRole NewRole)
{
	AssignedRole = NewRole;
	OnRoleChanged.Broadcast(NewRole);

	if (AFarmerAIController* FAI = Cast<AFarmerAIController>(GetController()))
	{
		FAI->ResetToIdle();
	}
}

UToolDataAsset* AFarmerVillagerCharacter::FindEquippedToolForRole() const
{
	if (!InventoryComponent)
		return nullptr;

	const EToolType RequiredToolType = ResolveRoleToolType(AssignedRole);
	if (RequiredToolType == EToolType::None)
		return nullptr;

	for (const FInventorySlot& Slot : InventoryComponent->GetInventorySlots())
	{
		if (Slot.IsEmpty())
			continue;

		if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(Slot.ItemDefinition.Get()))
		{
			if (ToolData->ToolType == RequiredToolType)
			{
				return const_cast<UToolDataAsset*>(ToolData);
			}
		}
	}

	return nullptr;
}

USeedDataAsset* AFarmerVillagerCharacter::FindSeedInInventory(int32& OutSlotIndex) const
{
	OutSlotIndex = INDEX_NONE;

	if (!InventoryComponent)
		return nullptr;

	const TArray<FInventorySlot>& Slots = InventoryComponent->GetInventorySlots();
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].IsEmpty())
			continue;

		if (const USeedDataAsset* SeedData = Cast<const USeedDataAsset>(Slots[i].ItemDefinition.Get()))
		{
			OutSlotIndex = i;
			return const_cast<USeedDataAsset*>(SeedData);
		}
	}

	return nullptr;
}

EToolType AFarmerVillagerCharacter::ResolveRoleToolType(EFarmerRole FarmerRole) const
{
	switch (FarmerRole)
	{
	case EFarmerRole::Harvester:
		return EToolType::Scythe;
	case EFarmerRole::Planter:
		return EToolType::Hoe;
	case EFarmerRole::Waterer:
		return EToolType::WateringCan;
	default:
		return EToolType::None;
	}
}

void AFarmerVillagerCharacter::OnCropHarvestedForMilestone(AActor* Harvester, UCropDataAsset* CropData, int32 Quantity)
{
	if (UWorld* World = GetWorld())
	{
		if (UMilestoneSubsystem* MilestoneSubsystem = World->GetSubsystem<UMilestoneSubsystem>())
		{
			MilestoneSubsystem->ReportEvent(TEXT("CropsHarvested"), Quantity);
		}
	}
}

void AFarmerVillagerCharacter::OnSeedPlantedForMilestone(AActor* Planter, USeedDataAsset* SeedData)
{
	if (UWorld* World = GetWorld())
	{
		if (UMilestoneSubsystem* MilestoneSubsystem = World->GetSubsystem<UMilestoneSubsystem>())
		{
			MilestoneSubsystem->ReportEvent(TEXT("SeedsPlanted"));
		}
	}
}
