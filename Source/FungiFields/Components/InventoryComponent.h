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
 * Component responsible for managing player inventory.
 * Handles item storage, stacking, and slot management.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryAddItem(UItemDataAsset* ItemToAdd, int32 Amount = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetMaxSlots() const { return InventorySlots.Num(); }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FInventorySlot>& GetInventorySlots() const { return InventorySlots; }

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

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 InitialSlotCount = 9;

	UPROPERTY(VisibleAnywhere, Category = "Inventory Data")
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Data")
	int32 CurrentEquippedSlotIndex = INDEX_NONE;

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

