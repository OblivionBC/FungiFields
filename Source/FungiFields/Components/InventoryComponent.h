#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "InventoryComponent.generated.h"

struct FInputActionValue;
class UItemDataAsset;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

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

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetEquippedSlot() const { return CurrentEquippedSlotIndex; }

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

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
	 * Attaches or removes mesh based on equipped item.
	 */
	void UpdateEquippedItemMesh();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> EquippedItemMeshComponent;
};

