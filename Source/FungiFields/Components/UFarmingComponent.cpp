#include "UFarmingComponent.h"
#include "Camera/CameraComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "../Data/UToolDataAsset.h"
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
	// No-op if no valid tool is equipped
	if (!bHasValidTool)
	{
		return;
	}

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
	if (!GetOwner())
	{
		bHasValidTool = false;
		return;
	}

	UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComp)
	{
		bHasValidTool = false;
		return;
	}

	int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
	if (EquippedSlotIndex == INDEX_NONE)
	{
		// No slot equipped
		bHasValidTool = false;
		CurrentToolType = EToolType::Hoe;
		CurrentToolPower = 1.0f;
		return;
	}

	const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
	if (!Slots.IsValidIndex(EquippedSlotIndex))
	{
		bHasValidTool = false;
		return;
	}

	const FInventorySlot& EquippedSlot = Slots[EquippedSlotIndex];
	if (EquippedSlot.IsEmpty())
	{
		// Slot is empty
		bHasValidTool = false;
		CurrentToolType = EToolType::Hoe;
		CurrentToolPower = 1.0f;
		return;
	}

	// Check if equipped item is a tool
	if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(EquippedSlot.ItemDefinition))
	{
		// Valid tool equipped
		bHasValidTool = true;
		CurrentToolType = ToolData->ToolType;
		CurrentToolPower = ToolData->ToolPower;
		ToolTraceDistance = ToolData->ToolRange;
	}
	else
	{
		// Item is not a tool - no-op
		bHasValidTool = false;
		CurrentToolType = EToolType::Hoe; // Keep for display purposes, but won't be used
		CurrentToolPower = 1.0f;
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