#include "InventoryComponent.h"
#include "../Data/UItemDataAsset.h"

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
	OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::EquipSlot(int32 SlotIndex)
{
	const int32 HotbarSize = 9;
	
	if (SlotIndex < 0 || SlotIndex >= HotbarSize)
	{
		return false;
	}

	if (SlotIndex >= InventorySlots.Num())
	{
		return false;
	}

	CurrentEquippedSlotIndex = SlotIndex;
	BroadcastUpdate();
	return true;
}
