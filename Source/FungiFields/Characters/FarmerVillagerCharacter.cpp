#include "FarmerVillagerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "../Components/UFarmingComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Components/AI/FarmerTargetingComponent.h"
#include "../Actors/ASoilPlot.h"
#include "../Components/UVillagerNeedsComponent.h"
#include "../Components/UBT_DialogueComponent.h"
#include "../Widgets/UBT_DialogueWidget.h"
#include "../AI/FarmerAIController.h"
#include "../Data/UToolDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "../Data/UCropDataAsset.h"
#include "../Data/UItemDataAsset.h"
#include "../Inventory/FInventorySlot.h"
#include "../Widgets/UVillagerManagementWidget.h"
#include "../Subsystems/UMilestoneSubsystem.h"

AFarmerVillagerCharacter::AFarmerVillagerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	FarmingComponent = CreateDefaultSubobject<UFarmingComponent>(TEXT("FarmingComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	TargetingComponent = CreateDefaultSubobject<UFarmerTargetingComponent>(TEXT("TargetingComponent"));
	NeedsComponent = CreateDefaultSubobject<UVillagerNeedsComponent>(TEXT("NeedsComponent"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AFarmerAIController::StaticClass();

	GetCharacterMovement()->MaxWalkSpeed = 280.0f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	// The default Pawn collision preset ignores ECC_Visibility, which makes line/sweep
	// traces from UInteractionComponent miss the villager capsule. Block it explicitly.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
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

	if (UBT_DialogueComponent* DialogueComp = FindComponentByClass<UBT_DialogueComponent>())
	{
		DialogueComp->OnManageOptionSelected.AddDynamic(this, &AFarmerVillagerCharacter::OnDialogueManageRequested);
	}
}

void AFarmerVillagerCharacter::Interact_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("AFarmerVillagerCharacter::Interact_Implementation called on '%s'"), *GetName());

	if (!Interactor)
	{
		UE_LOG(LogTemp, Warning, TEXT("  -> Interactor is null, returning."));
		return;
	}

	// Feeding takes priority: if the player is holding food, feed the villager and exit.
	if (TryFeedFromInteractor(Interactor))
	{
		UE_LOG(LogTemp, Log, TEXT("  -> Fed villager with held food item."));
		return;
	}

	APawn* InteractorPawn = Cast<APawn>(Interactor);
	APlayerController* PC = InteractorPawn ? Cast<APlayerController>(InteractorPawn->GetController()) : nullptr;
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("  -> Could not get PlayerController from interactor '%s', returning."), *Interactor->GetName());
		return;
	}

	LastInteractorPC = PC;

	if (UBT_DialogueComponent* DialogueComp = FindComponentByClass<UBT_DialogueComponent>())
	{
		if (DialogueComp->DialogueWidgetClass)
		{
			UE_LOG(LogTemp, Log, TEXT("  -> Delegating to UBT_DialogueComponent."));
			DialogueComp->InitiateDialogue(Interactor);
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("  -> UBT_DialogueComponent found but DialogueWidgetClass is null. Falling through to OpenManagementWidget."));
	}

	UE_LOG(LogTemp, Log, TEXT("  -> Calling OpenManagementWidget. VillagerManagementWidgetClass set: %s"),
		VillagerManagementWidgetClass ? TEXT("YES") : TEXT("NO — widget will NOT open"));
	OpenManagementWidget(PC);
}

bool AFarmerVillagerCharacter::TryFeedFromInteractor(AActor* Interactor)
{
	if (!Interactor || !NeedsComponent) return false;

	UInventoryComponent* InvComp = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!InvComp) return false;

	const int32 EquippedSlot = InvComp->GetEquippedSlot();
	if (EquippedSlot == INDEX_NONE) return false;

	const TArray<FInventorySlot>& Slots = InvComp->GetInventorySlots();
	if (!Slots.IsValidIndex(EquippedSlot) || Slots[EquippedSlot].IsEmpty()) return false;

	const UItemDataAsset* ItemDef = Slots[EquippedSlot].ItemDefinition.Get();
	if (!ItemDef || !ItemDef->bIsFood) return false;

	NeedsComponent->Feed(ItemDef->FoodRestoreAmount);
	InvComp->ConsumeFromSlot(EquippedSlot, 1);
	return true;
}

void AFarmerVillagerCharacter::OnDialogueManageRequested()
{
	// When using UBT_DialogueComponent, prefer its cached PC; otherwise fall back
	// to the one stored at the start of Interact_Implementation.
	APlayerController* PC = nullptr;

	if (UBT_DialogueComponent* DialogueComp = FindComponentByClass<UBT_DialogueComponent>())
	{
		PC = DialogueComp->GetCachedController();
	}

	if (!PC)
	{
		PC = LastInteractorPC.Get();
	}

	if (PC)
	{
		OpenManagementWidget(PC);
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
	// Show "Feed" prompt when the player is holding a food item.
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				if (UInventoryComponent* InvComp = Pawn->FindComponentByClass<UInventoryComponent>())
				{
					const int32 EquippedSlot = InvComp->GetEquippedSlot();
					const TArray<FInventorySlot>& Slots = InvComp->GetInventorySlots();
					if (Slots.IsValidIndex(EquippedSlot) && !Slots[EquippedSlot].IsEmpty())
					{
						if (const UItemDataAsset* Item = Slots[EquippedSlot].ItemDefinition.Get())
						{
							if (Item->bIsFood)
								return FText::Format(INVTEXT("Feed {0}"), VillagerDisplayName);
						}
					}
				}
			}
		}
	}
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

bool AFarmerVillagerCharacter::AssignPlot(ASoilPlot* Plot)
{
	if (!IsValid(Plot)) return false;
	if (IsPlotAssigned(Plot)) return false;

	AssignedPlots.RemoveAll([](const TWeakObjectPtr<ASoilPlot>& P) { return !P.IsValid(); });

	const int32 MaxBeds = NeedsComponent ? NeedsComponent->GetMaxAssignedCropBeds() : 3;
	if (AssignedPlots.Num() >= MaxBeds) return false;

	AssignedPlots.Add(Plot);
	OnAssignedPlotsChanged.Broadcast();
	return true;
}

void AFarmerVillagerCharacter::UnassignPlot(ASoilPlot* Plot)
{
	if (!IsValid(Plot)) return;
	const int32 Removed = AssignedPlots.RemoveAll(
		[Plot](const TWeakObjectPtr<ASoilPlot>& P) { return P.Get() == Plot; });
	if (Removed > 0)
		OnAssignedPlotsChanged.Broadcast();
}

bool AFarmerVillagerCharacter::IsPlotAssigned(const ASoilPlot* Plot) const
{
	for (const TWeakObjectPtr<ASoilPlot>& P : AssignedPlots)
	{
		if (P.Get() == Plot) return true;
	}
	return false;
}

int32 AFarmerVillagerCharacter::GetAssignedPlotCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ASoilPlot>& P : AssignedPlots)
	{
		if (P.IsValid()) ++Count;
	}
	return Count;
}
