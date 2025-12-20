#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "InventoryComponent.generated.h"

struct FInputActionValue;
class UItemDataAsset;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemAdded, UItemDataAsset*, Item, int32, Amount, int32, NewTotal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemRemoved, UItemDataAsset*, Item, int32, Amount, int32, NewTotal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquipped, UItemDataAsset*, Item, int32, SlotIndex);

/**
 * Generic inventory component that can be used for players, chests, or any actor.
 * Handles item storage, stacking, and slot management.
 * Supports optional equipping (for characters) and optional replication (for multiplayer).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryAddItem(UItemDataAsset* ItemToAdd, int32 Amount = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetMaxSlots() const { return InventorySlots.Num(); }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FInventorySlot>& GetInventorySlots() const { return InventorySlots; }

	/**
	 * Equip an item from a slot. Only works if bSupportsEquipping is true.
	 * @param Value Input action value (can be empty if called directly)
	 * @param SlotIndex Index of the slot to equip
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void EquipSlot(const FInputActionValue& Value, int32 SlotIndex);

	/**
	 * Consume items from a specific inventory slot.
	 * Intended to be called on the server in a multiplayer scenario.
	 * @param SlotIndex Index of the slot to consume from
	 * @param Amount Number of items to consume
	 * @return True if any items were consumed
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ConsumeFromSlot(int32 SlotIndex, int32 Amount);

	/**
	 * Move an item from one slot to another within the inventory.
	 * @param FromSlotIndex Source slot index
	 * @param ToSlotIndex Destination slot index
	 * @return True if the move was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItemToSlot(int32 FromSlotIndex, int32 ToSlotIndex);

	/**
	 * Swap items between two slots in the inventory.
	 * @param SlotAIndex First slot index
	 * @param SlotBIndex Second slot index
	 * @return True if the swap was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SwapSlots(int32 SlotAIndex, int32 SlotBIndex);

	/**
	 * Remove items from a specific slot.
	 * @param SlotIndex Index of the slot to remove from
	 * @param Amount Number of items to remove
	 * @return True if any items were removed
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveFromSlot(int32 SlotIndex, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetEquippedSlot() const { return CurrentEquippedSlotIndex; }

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	/** Delegate broadcast when an item is added to inventory */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;

	/** Delegate broadcast when an item is removed from inventory */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;

	/** Delegate broadcast when an item is equipped */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemEquipped OnItemEquipped;

	/** Set the initial slot count (call before BeginPlay) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInitialSlotCount(int32 SlotCount) { InitialSlotCount = SlotCount; }

	/** Enable or disable replication (must be called in constructor, before component registration) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetEnableReplication(bool bEnable);

	/** Enable or disable equipping support (call before BeginPlay) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSupportsEquipping(bool bSupport) { bSupportsEquipping = bSupport; }

protected:
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 InitialSlotCount = 27;

	/** Whether this inventory should support replication (for multiplayer) 
	 *  Note: If set to true, replication will be enabled during component construction.
	 *  This can be set via EditDefaultsOnly in Blueprint, or by directly setting this property
	 *  immediately after CreateDefaultSubobject (before PostInitProperties runs).
	 *  WARNING: SetEnableReplication() cannot enable replication if called after PostInitProperties.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory", meta = (AllowPrivateAccess = "false"))
	bool bEnableReplication = false;

	/** Whether this inventory supports equipping items (typically only for player characters) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	bool bSupportsEquipping = true;

	UPROPERTY(ReplicatedUsing = OnRep_InventorySlots, VisibleAnywhere, Category = "Inventory Data")
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Data")
	int32 CurrentEquippedSlotIndex = INDEX_NONE;

	UFUNCTION()
	void OnRep_InventorySlots();

private:
	bool TryStackItem(UItemDataAsset* ItemToAdd, int32& RemainingAmount);

	bool AddToNewSlot(UItemDataAsset* ItemToAdd, int32 Amount);

	void BroadcastUpdate();

	/**
	 * Get the total count of a specific item across all inventory slots.
	 * @param Item The item to count
	 * @return Total quantity of the item in inventory
	 */
	int32 GetItemTotalCount(UItemDataAsset* Item) const;

	/**
	 * Attaches or removes mesh based on equipped item.
	 */
	void UpdateEquippedItemMesh();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> EquippedItemMeshComponent;
};

