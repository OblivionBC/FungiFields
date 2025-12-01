#include "InventoryComponent.h"
#include "../Data/UItemDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

struct FInputActionValue;

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentEquippedSlotIndex(INDEX_NONE)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventorySlots.SetNum(InitialSlotCount);
}

bool UInventoryComponent::TryAddItem(UItemDataAsset* ItemToAdd, int32 Amount)
{
	if (!ItemToAdd || Amount <= 0)
	{
		return false;
	}

	int32 RemainingAmount = Amount;
	bool bAnyItemAdded = false;

	if (TryStackItem(ItemToAdd, RemainingAmount))
	{
		bAnyItemAdded = true;
		BroadcastUpdate();
	}

	if (RemainingAmount > 0)
	{
		if (AddToNewSlot(ItemToAdd, RemainingAmount))
		{
			bAnyItemAdded = true;
			BroadcastUpdate();
		}
	}

	return bAnyItemAdded;
}

bool UInventoryComponent::TryStackItem(UItemDataAsset* ItemToAdd, int32& RemainingAmount)
{
	if (!ItemToAdd || RemainingAmount <= 0)
	{
		return false;
	}

	bool bStackedAny = false;

	for (FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.ItemDefinition == ItemToAdd && Slot.Count < ItemToAdd->MaxStackSize)
		{
			int32 SpaceAvailable = ItemToAdd->MaxStackSize - Slot.Count;
			int32 AmountToAdd = FMath::Min(RemainingAmount, SpaceAvailable);

			Slot.Count += AmountToAdd;
			RemainingAmount -= AmountToAdd;
			bStackedAny = true;

			if (RemainingAmount <= 0)
			{
				break;
			}
		}
	}

	return bStackedAny;
}

bool UInventoryComponent::AddToNewSlot(UItemDataAsset* ItemToAdd, int32 Amount)
{
	if (!ItemToAdd || Amount <= 0)
	{
		return false;
	}

	for (FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.IsEmpty())
		{
			// Clamp amount to max stack size
			int32 AmountToAdd = FMath::Min(Amount, ItemToAdd->MaxStackSize);
			Slot.ItemDefinition = ItemToAdd;
			Slot.Count = AmountToAdd;
			return true;
		}
	}

	return false;
}

void UInventoryComponent::BroadcastUpdate()
{
	// Update equipped item mesh if equipped slot might have changed
	UpdateEquippedItemMesh();
	OnInventoryChanged.Broadcast();
}

void UInventoryComponent::UpdateEquippedItemMesh()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Get character mesh for socket attachment
	ACharacter* CharacterOwner = Cast<ACharacter>(Owner);
	if (!CharacterOwner)
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = CharacterOwner->GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	// Remove existing equipped item mesh
	if (EquippedItemMeshComponent)
	{
		EquippedItemMeshComponent->DestroyComponent();
		EquippedItemMeshComponent = nullptr;
	}

	// Check if we have an equipped slot
	if (CurrentEquippedSlotIndex == INDEX_NONE)
	{
		return;
	}

	// Check if slot is valid
	if (!InventorySlots.IsValidIndex(CurrentEquippedSlotIndex))
	{
		return;
	}

	const FInventorySlot& EquippedSlot = InventorySlots[CurrentEquippedSlotIndex];
	if (EquippedSlot.IsEmpty() || !EquippedSlot.ItemDefinition)
	{
		return;
	}

	// Check if item has a mesh
	UStaticMesh* ItemMesh = EquippedSlot.ItemDefinition->ItemMesh.LoadSynchronous();
	if (!ItemMesh)
	{
		return;
	}

	// Create static mesh component for the equipped item
	EquippedItemMeshComponent = NewObject<UStaticMeshComponent>(Owner, UStaticMeshComponent::StaticClass(), TEXT("EquippedItemMesh"));
	if (!EquippedItemMeshComponent)
	{
		return;
	}

	// Register and initialize the component
	EquippedItemMeshComponent->RegisterComponent();
	EquippedItemMeshComponent->SetStaticMesh(ItemMesh);
	EquippedItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedItemMeshComponent->SetGenerateOverlapEvents(false);

	// Attach to character mesh at RightHandItemSlot socket
	EquippedItemMeshComponent->AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("RightHandItemSlot")
	);
}

void UInventoryComponent::EquipSlot(const FInputActionValue& Value, int32 SlotIndex)
{
	const int32 HotbarSize = 9;
	
	if (SlotIndex < 0 || SlotIndex >= HotbarSize || SlotIndex >= InventorySlots.Num())
	{
		return;
	}

	CurrentEquippedSlotIndex = SlotIndex;
	UpdateEquippedItemMesh();
	BroadcastUpdate();
}

bool UInventoryComponent::ConsumeFromSlot(int32 SlotIndex, int32 Amount)
{
	// This function is intended to run on the server in a multiplayer scenario.
	if (Amount <= 0)
	{
		return false;
	}

	if (!InventorySlots.IsValidIndex(SlotIndex))
	{
		return false;
	}

	FInventorySlot& Slot = InventorySlots[SlotIndex];
	if (Slot.IsEmpty())
	{
		return false;
	}

	const int32 OriginalCount = Slot.Count;
	Slot.Count = FMath::Max(0, Slot.Count - Amount);

	if (Slot.Count == 0)
	{
		Slot.ItemDefinition = nullptr;
	}

	if (Slot.Count == OriginalCount)
	{
		return false;
	}

	// If the consumed slot is equipped, ensure visuals/state are refreshed.
	if (CurrentEquippedSlotIndex == SlotIndex)
	{
		UpdateEquippedItemMesh();
	}

	BroadcastUpdate();
	return true;
}