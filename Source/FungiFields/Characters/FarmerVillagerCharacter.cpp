#include "FarmerVillagerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Components/UFarmingComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Components/AI/FarmerTargetingComponent.h"
#include "../AI/FarmerAIController.h"

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

	if (FarmingComponent)
	{
		// NPCs execute via location-based farming API, so no camera setup is required.
		FarmingComponent->SetEquippedTool(ResolveRoleToolType(AssignedRole), RoleToolPower);
	}
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
