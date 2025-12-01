#include "UFarmingComponent.h"
#include "Camera/CameraComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "../Data/UToolDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "../Interfaces/IFarmableInterface.h"
#include "../Interfaces/IHarvestableInterface.h"
#include "../Data/FHarvestResult.h"
#include "../Attributes/CharacterAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UFarmingComponent::UFarmingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentToolType = EToolType::Hoe;
	CurrentToolPower = 1.0f;
	bHasValidTool = false;
	bHasSeedEquipped = false;
	EquippedSeedData = nullptr;
	EquippedSlotIndexCached = INDEX_NONE;
}

void UFarmingComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateEquippedTool();
}

void UFarmingComponent::SetCamera(UCameraComponent* Camera)
{
	CameraComponent = Camera;
}

void UFarmingComponent::UseEquippedTool(const FInputActionValue& Value)
{
	if (!CameraComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFarmingComponent::UseEquippedTool: CameraComponent is null!"));
		return;
	}

	// Perform line trace
	FHitResult HitResult;
	if (!PerformToolTrace(HitResult))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return;
	}

	// if a seed is equipped and we're looking at farmable soil, try to plant.
	if (bHasSeedEquipped && EquippedSeedData && EquippedSeedData->CropToPlant && HitActor->Implements<UFarmableInterface>())
	{
		if (IFarmableInterface::Execute_CanAcceptSeed(HitActor))
		{
			if (IFarmableInterface::Execute_PlantSeed(HitActor, EquippedSeedData->CropToPlant, GetOwner()))
			{
				if (UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>())
				{
					// Consume one seed from the equipped slot on the authoritative inventory owner.
					if (InventoryComp->ConsumeFromSlot(EquippedSlotIndexCached, 1))
					{
						// Re-evaluate equipped state after inventory change.
						UpdateEquippedTool();
					}
				}
				// Plant Stamina Usage here in future
				return;
			}
		}
	}

	if (!bHasValidTool)
	{
		return;
	}

	// Check if actor implements IFarmableInterface
	if (HitActor->Implements<UFarmableInterface>())
	{
		bool bSuccess = IFarmableInterface::Execute_InteractTool(HitActor, CurrentToolType, GetOwner(), CurrentToolPower);
		
		if (bSuccess)
		{
			// Consume stamina - get cost from current tool
			float StaminaCost = 10.0f; // Default
			
			// Try to get actual stamina cost from equipped tool
			UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();
			if (InventoryComp)
			{
				int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
				if (EquippedSlotIndex != INDEX_NONE)
				{
					const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
					if (Slots.IsValidIndex(EquippedSlotIndex))
					{
						const FInventorySlot& Slot = Slots[EquippedSlotIndex];
						if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(Slot.ItemDefinition))
						{
							StaminaCost = ToolData->StaminaCost;
						}
					}
				}
			}
			
			ConsumeStamina(StaminaCost);
		}
	}
	// Check if actor implements IHarvestableInterface
	else if (HitActor->Implements<UHarvestableInterface>())
	{
		if (IHarvestableInterface::Execute_CanHarvest(HitActor))
		{
			FHarvestResult HarvestResult = IHarvestableInterface::Execute_Harvest(HitActor, GetOwner(), CurrentToolPower);
			
			if (HarvestResult.bSuccess)
			{
				// Consume stamina - get cost from current tool
				float StaminaCost = 10.0f;
				
				// Try to get actual stamina cost from equipped tool
				UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();
				if (InventoryComp)
				{
					int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
					if (EquippedSlotIndex != INDEX_NONE)
					{
						const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
						if (Slots.IsValidIndex(EquippedSlotIndex))
						{
							const FInventorySlot& Slot = Slots[EquippedSlotIndex];
							if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(Slot.ItemDefinition))
							{
								StaminaCost = ToolData->StaminaCost;
							}
						}
					}
				}
				
				ConsumeStamina(StaminaCost);
			}
		}
	}
}

void UFarmingComponent::SetEquippedTool(EToolType ToolType, float ToolPower)
{
	CurrentToolType = ToolType;
	CurrentToolPower = ToolPower;
}

void UFarmingComponent::UpdateEquippedTool()
{
	bHasValidTool = false;
	bHasSeedEquipped = false;
	EquippedSeedData = nullptr;
	EquippedSlotIndexCached = INDEX_NONE;

	if (!GetOwner())
	{
		return;
	}

	UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComp)
	{
		return;
	}

	const int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
	if (EquippedSlotIndex == INDEX_NONE)
	{
		return;
	}

	const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
	if (!Slots.IsValidIndex(EquippedSlotIndex))
	{
		return;
	}

	const FInventorySlot& EquippedSlot = Slots[EquippedSlotIndex];
	if (EquippedSlot.IsEmpty())
	{
		return;
	}

	EquippedSlotIndexCached = EquippedSlotIndex;

	// Check if equipped item is a tool
	if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(EquippedSlot.ItemDefinition))
	{
		// Valid tool equipped
		bHasValidTool = true;
		CurrentToolType = ToolData->ToolType;
		CurrentToolPower = ToolData->ToolPower;
		ToolTraceDistance = ToolData->ToolRange;
		return;
	}

	if (USeedDataAsset* SeedData = const_cast<USeedDataAsset*>(Cast<const USeedDataAsset>(EquippedSlot.ItemDefinition)))
	{
		if (SeedData->CropToPlant)
		{
			bHasSeedEquipped = true;
			EquippedSeedData = SeedData;
		}
	}
}

bool UFarmingComponent::PerformToolTrace(FHitResult& OutHit) const
{
	if (!CameraComponent || !GetWorld())
	{
		return false;
	}

	FVector Start = CameraComponent->GetComponentLocation();
	FVector ForwardVector = CameraComponent->GetForwardVector();
	FVector End = Start + (ForwardVector * ToolTraceDistance);

	FCollisionQueryParams TraceParams(FName(TEXT("ToolTrace")), true, GetOwner());
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		TraceParams
	);

	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 2.0f);

	return bHit;
}

bool UFarmingComponent::ConsumeStamina(float StaminaCost)
{
	if (!GetOwner())
	{
		return false;
	}

	// Get ability system component
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
		if (!ASC)
		{
			return false;
		}

		// Get character attribute set
		const UCharacterAttributeSet* AttributeSet = ASC->GetSet<UCharacterAttributeSet>();
		if (!AttributeSet)
		{
			return false;
		}

		// Check if enough stamina
		float CurrentStamina = AttributeSet->GetStamina();
		if (CurrentStamina < StaminaCost)
		{
			return false;
		}

		// Consume stamina
		ASC->ApplyModToAttribute(
			UCharacterAttributeSet::GetStaminaAttribute(),
			EGameplayModOp::Additive,
			-StaminaCost
		);

		return true;
	}

	return false;
}