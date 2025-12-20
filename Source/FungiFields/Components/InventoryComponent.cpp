#include "InventoryComponent.h"
#include "../Data/UItemDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

struct FInputActionValue;

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bEnableReplication(false)
	, bSupportsEquipping(true)
	, CurrentEquippedSlotIndex(INDEX_NONE)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::PostInitProperties()
{
	Super::PostInitProperties();
	
	// SetIsReplicatedByDefault must be called during construction.
	// PostInitProperties is called during construction, so we can set it here.
	// This handles the case where bEnableReplication is set via EditDefaultsOnly in Blueprint.
	// 
	// IMPORTANT: If bEnableReplication is set AFTER PostInitProperties runs (e.g., from actor constructor),
	// replication will NOT be enabled. The flag must be set BEFORE PostInitProperties runs.
	// This means either:
	// 1. Set it via EditDefaultsOnly in Blueprint (before component creation)
	// 2. Set it immediately after CreateDefaultSubobject but before PostInitProperties runs (not possible)
	// 3. Create a custom component class with bEnableReplication = true by default
	//
	// For now, we check the flag here. If it's true, enable replication.
	if (bEnableReplication)
	{
		SetIsReplicatedByDefault(true);
	}
}

void UInventoryComponent::SetEnableReplication(bool bEnable)
{
	// Set the flag - but DO NOT call SetIsReplicatedByDefault here!
	// SetIsReplicatedByDefault can ONLY be called during component construction.
	// If this is called from actor constructor after CreateDefaultSubobject, PostInitProperties
	// has already run, so we cannot enable replication this way.
	// 
	// Solution: Set bEnableReplication BEFORE CreateDefaultSubobject, or use EditDefaultsOnly in Blueprint.
	bEnableReplication = bEnable;
	
	// We cannot call SetIsReplicatedByDefault here because construction may have already completed.
	// This function should only be used to set the flag before component creation, or it won't work.
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	if (bEnableReplication)
	{
		DOREPLIFETIME(UInventoryComponent, InventorySlots);
	}
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventorySlots.SetNum(InitialSlotCount);

	// Auto-detect if equipping should be enabled based on owner type
	if (bSupportsEquipping && GetOwner())
	{
		// Only enable equipping if owner is a character
		bSupportsEquipping = Cast<ACharacter>(GetOwner()) != nullptr;
	}
}

bool UInventoryComponent::TryAddItem(UItemDataAsset* ItemToAdd, int32 Amount)
{
	if (!ItemToAdd || Amount <= 0)
	{
		return false;
	}

	// If replication is enabled, only allow modifications on server
	if (bEnableReplication && GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	int32 RemainingAmount = Amount;
	bool bAnyItemAdded = false;
	int32 TotalAdded = 0;

	if (TryStackItem(ItemToAdd, RemainingAmount))
	{
		bAnyItemAdded = true;
		TotalAdded = Amount - RemainingAmount;
		BroadcastUpdate();
	}

	if (RemainingAmount > 0)
	{
		if (AddToNewSlot(ItemToAdd, RemainingAmount))
		{
			bAnyItemAdded = true;
			TotalAdded += RemainingAmount;
			BroadcastUpdate();
		}
	}

	if (bAnyItemAdded)
	{
		// Calculate new total quantity of this item
		int32 NewTotal = GetItemTotalCount(ItemToAdd);
		
		// Broadcast specific item added event
		OnItemAdded.Broadcast(ItemToAdd, TotalAdded, NewTotal);
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
	// Update equipped item mesh if equipped slot might have changed (only if equipping is supported)
	if (bSupportsEquipping)
	{
		UpdateEquippedItemMesh();
	}
	OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_InventorySlots()
{
	BroadcastUpdate();
}

void UInventoryComponent::UpdateEquippedItemMesh()
{
	if (!bSupportsEquipping)
	{
		return;
	}

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
	UStaticMesh* ItemMesh = EquippedSlot.ItemDefinition->ItemMesh;
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

	// Set up the component properties before registration
	EquippedItemMeshComponent->SetStaticMesh(ItemMesh);
	EquippedItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedItemMeshComponent->SetGenerateOverlapEvents(false);
	EquippedItemMeshComponent->SetVisibility(true);
	EquippedItemMeshComponent->SetHiddenInGame(false);

	// Attach to character mesh at RightHandItemSlot socket BEFORE registration
	// This ensures the component is properly parented in the component hierarchy
	EquippedItemMeshComponent->AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("RightHandItemSlot")
	);

	// Register the component after attachment to ensure it's in the correct hierarchy
	EquippedItemMeshComponent->RegisterComponent();
}

void UInventoryComponent::EquipSlot(const FInputActionValue& Value, int32 SlotIndex)
{
	if (!bSupportsEquipping)
	{
		return;
	}

	const int32 HotbarSize = 9;
	
	if (SlotIndex < 0 || SlotIndex >= HotbarSize || SlotIndex >= InventorySlots.Num())
	{
		return;
	}

	CurrentEquippedSlotIndex = SlotIndex;
	
	// Get the item being equipped
	UItemDataAsset* EquippedItem = nullptr;
	if (InventorySlots.IsValidIndex(SlotIndex) && !InventorySlots[SlotIndex].IsEmpty())
	{
		EquippedItem = const_cast<UItemDataAsset*>(InventorySlots[SlotIndex].ItemDefinition.Get());
	}
	
	UpdateEquippedItemMesh();
	
	// Broadcast item equipped event
	if (EquippedItem)
	{
		OnItemEquipped.Broadcast(EquippedItem, SlotIndex);
	}
	
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
	const UItemDataAsset* ItemToRemove = Slot.ItemDefinition;
	const int32 AmountToRemove = FMath::Min(Amount, OriginalCount);
	
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
	if (bSupportsEquipping && CurrentEquippedSlotIndex == SlotIndex)
	{
		UpdateEquippedItemMesh();
	}

	// Calculate new total quantity
	int32 NewTotal = GetItemTotalCount(const_cast<UItemDataAsset*>(ItemToRemove));
	
	// Broadcast specific item removed event
	OnItemRemoved.Broadcast(const_cast<UItemDataAsset*>(ItemToRemove), AmountToRemove, NewTotal);

	BroadcastUpdate();
	return true;
}

bool UInventoryComponent::RemoveFromSlot(int32 SlotIndex, int32 Amount)
{
	// If replication is enabled, only allow modifications on server
	if (bEnableReplication && GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

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

	const int32 AmountToRemove = FMath::Min(Amount, Slot.Count);
	Slot.Count -= AmountToRemove;

	if (Slot.Count <= 0)
	{
		Slot.ItemDefinition = nullptr;
		Slot.Count = 0;
	}

	// If the removed slot is equipped, ensure visuals/state are refreshed.
	if (bSupportsEquipping && CurrentEquippedSlotIndex == SlotIndex)
	{
		UpdateEquippedItemMesh();
	}

	BroadcastUpdate();
	return true;
}

int32 UInventoryComponent::GetItemTotalCount(UItemDataAsset* Item) const
{
	if (!Item)
	{
		return 0;
	}

	int32 TotalCount = 0;
	for (const FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.ItemDefinition == Item)
		{
			TotalCount += Slot.Count;
		}
	}

	return TotalCount;
}

bool UInventoryComponent::MoveItemToSlot(int32 FromSlotIndex, int32 ToSlotIndex)
{
	// If replication is enabled, only allow modifications on server
	if (bEnableReplication && GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	if (!InventorySlots.IsValidIndex(FromSlotIndex) || !InventorySlots.IsValidIndex(ToSlotIndex))
	{
		return false;
	}

	if (FromSlotIndex == ToSlotIndex)
	{
		return false;
	}

	FInventorySlot& FromSlot = InventorySlots[FromSlotIndex];
	FInventorySlot& ToSlot = InventorySlots[ToSlotIndex];

	if (FromSlot.IsEmpty())
	{
		return false;
	}

	// If target slot is empty, move the item
	if (ToSlot.IsEmpty())
	{
		ToSlot = FromSlot;
		FromSlot.ItemDefinition = nullptr;
		FromSlot.Count = 0;
		
		// Update equipped slot if needed (only if equipping is supported)
		if (bSupportsEquipping)
		{
			if (CurrentEquippedSlotIndex == FromSlotIndex)
			{
				CurrentEquippedSlotIndex = ToSlotIndex;
			}
			else if (CurrentEquippedSlotIndex == ToSlotIndex)
			{
				// Item moved to equipped slot, update mesh
				UpdateEquippedItemMesh();
			}
		}
		
		BroadcastUpdate();
		return true;
	}

	// If same item, try to stack
	if (FromSlot.ItemDefinition == ToSlot.ItemDefinition)
	{
		int32 SpaceAvailable = ToSlot.ItemDefinition->MaxStackSize - ToSlot.Count;
		if (SpaceAvailable > 0)
		{
			int32 AmountToMove = FMath::Min(FromSlot.Count, SpaceAvailable);
			ToSlot.Count += AmountToMove;
			FromSlot.Count -= AmountToMove;

			if (FromSlot.Count <= 0)
			{
				FromSlot.ItemDefinition = nullptr;
				FromSlot.Count = 0;
			}

			// Update equipped slot if needed (only if equipping is supported)
			if (bSupportsEquipping)
			{
				if (CurrentEquippedSlotIndex == FromSlotIndex && FromSlot.IsEmpty())
				{
					UpdateEquippedItemMesh();
				}
				else if (CurrentEquippedSlotIndex == ToSlotIndex)
				{
					UpdateEquippedItemMesh();
				}
			}

			BroadcastUpdate();
			return true;
		}
	}

	// Otherwise, swap
	return SwapSlots(FromSlotIndex, ToSlotIndex);
}

bool UInventoryComponent::SwapSlots(int32 SlotAIndex, int32 SlotBIndex)
{
	// If replication is enabled, only allow modifications on server
	if (bEnableReplication && GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	if (!InventorySlots.IsValidIndex(SlotAIndex) || !InventorySlots.IsValidIndex(SlotBIndex))
	{
		return false;
	}

	if (SlotAIndex == SlotBIndex)
	{
		return false;
	}

	FInventorySlot Temp = InventorySlots[SlotAIndex];
	InventorySlots[SlotAIndex] = InventorySlots[SlotBIndex];
	InventorySlots[SlotBIndex] = Temp;

	// Update equipped slot if needed (only if equipping is supported)
	if (bSupportsEquipping)
	{
		if (CurrentEquippedSlotIndex == SlotAIndex)
		{
			CurrentEquippedSlotIndex = SlotBIndex;
			UpdateEquippedItemMesh();
		}
		else if (CurrentEquippedSlotIndex == SlotBIndex)
		{
			CurrentEquippedSlotIndex = SlotAIndex;
			UpdateEquippedItemMesh();
		}
	}

	BroadcastUpdate();
	return true;
}