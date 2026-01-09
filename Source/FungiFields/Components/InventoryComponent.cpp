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
	
	if (bEnableReplication)
	{
		SetIsReplicatedByDefault(true);
	}
}

void UInventoryComponent::SetEnableReplication(bool bEnable)
{
	bEnableReplication = bEnable;
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

	if (bSupportsEquipping && GetOwner())
	{
		bSupportsEquipping = Cast<ACharacter>(GetOwner()) != nullptr;
	}
}

bool UInventoryComponent::TryAddItem(UItemDataAsset* ItemToAdd, int32 Amount)
{
	if (!ItemToAdd || Amount <= 0)
	{
		return false;
	}

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
		int32 NewTotal = GetItemTotalCount(ItemToAdd);
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

	if (EquippedItemMeshComponent)
	{
		EquippedItemMeshComponent->DestroyComponent();
		EquippedItemMeshComponent = nullptr;
	}

	if (CurrentEquippedSlotIndex == INDEX_NONE)
	{
		return;
	}

	if (!InventorySlots.IsValidIndex(CurrentEquippedSlotIndex))
	{
		return;
	}

	const FInventorySlot& EquippedSlot = InventorySlots[CurrentEquippedSlotIndex];
	if (EquippedSlot.IsEmpty() || !EquippedSlot.ItemDefinition)
	{
		return;
	}

	UStaticMesh* ItemMesh = EquippedSlot.ItemDefinition->ItemMesh;
	if (!ItemMesh)
	{
		return;
	}

	EquippedItemMeshComponent = NewObject<UStaticMeshComponent>(Owner, UStaticMeshComponent::StaticClass(), TEXT("EquippedItemMesh"));
	if (!EquippedItemMeshComponent)
	{
		return;
	}

	EquippedItemMeshComponent->SetStaticMesh(ItemMesh);
	EquippedItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedItemMeshComponent->SetGenerateOverlapEvents(false);
	EquippedItemMeshComponent->SetVisibility(true);
	EquippedItemMeshComponent->SetHiddenInGame(false);

	EquippedItemMeshComponent->AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("RightHandItemSlot")
	);

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
	
	UItemDataAsset* EquippedItem = nullptr;
	if (InventorySlots.IsValidIndex(SlotIndex) && !InventorySlots[SlotIndex].IsEmpty())
	{
		EquippedItem = const_cast<UItemDataAsset*>(InventorySlots[SlotIndex].ItemDefinition.Get());
	}
	
	UpdateEquippedItemMesh();
	
	if (EquippedItem)
	{
		OnItemEquipped.Broadcast(EquippedItem, SlotIndex);
	}
	
	BroadcastUpdate();
}

bool UInventoryComponent::ConsumeFromSlot(int32 SlotIndex, int32 Amount)
{
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

	if (bSupportsEquipping && CurrentEquippedSlotIndex == SlotIndex)
	{
		UpdateEquippedItemMesh();
	}

	int32 NewTotal = GetItemTotalCount(const_cast<UItemDataAsset*>(ItemToRemove));
	OnItemRemoved.Broadcast(const_cast<UItemDataAsset*>(ItemToRemove), AmountToRemove, NewTotal);

	BroadcastUpdate();
	return true;
}

bool UInventoryComponent::RemoveFromSlot(int32 SlotIndex, int32 Amount)
{
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

	if (ToSlot.IsEmpty())
	{
		ToSlot = FromSlot;
		FromSlot.ItemDefinition = nullptr;
		FromSlot.Count = 0;
		
		if (bSupportsEquipping)
		{
			if (CurrentEquippedSlotIndex == FromSlotIndex)
			{
				CurrentEquippedSlotIndex = ToSlotIndex;
			}
			else if (CurrentEquippedSlotIndex == ToSlotIndex)
			{
				UpdateEquippedItemMesh();
			}
		}
		
		BroadcastUpdate();
		return true;
	}

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

	return SwapSlots(FromSlotIndex, ToSlotIndex);
}

bool UInventoryComponent::SwapSlots(int32 SlotAIndex, int32 SlotBIndex)
{
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