#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/InteractableInterface.h"
#include "../Data/UShopStockDataAsset.h"
#include "AShopActor.generated.h"

class UInventoryComponent;
class UAbilitySystemComponent;
class UShopWidget;
class UStaticMeshComponent;
struct FInventorySlot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemPurchased, UItemDataAsset*, Item, int32, Quantity, float, TotalCost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemSold, UItemDataAsset*, Item, int32, Quantity, float, TotalProceeds);

/**
 * A placeable shop actor the player can interact with to buy and sell items.
 * All transactions go through GAS (Gold attribute) and UInventoryComponent,
 * so they automatically trigger any listening quest objectives.
 */
UCLASS()
class FUNGIFIELDS_API AShopActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AShopActor();

	// ~IInteractableInterface
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionText_Implementation() override;
	// ~ITooltipProvider (required by IInteractableInterface parent)
	virtual FText GetTooltipText_Implementation() const override;

	/**
	 * Attempts to purchase Quantity units of a stock entry on behalf of the player.
	 * Validates Gold balance, adds items to inventory, then deducts Gold via GAS.
	 * @return True if the purchase succeeded.
	 */
	bool TryBuyItem(UInventoryComponent* PlayerInventory, UAbilitySystemComponent* PlayerASC,
	                const FShopStockEntry& Entry, int32 Quantity);

	/**
	 * Attempts to sell Quantity units from a player inventory slot back to the shop.
	 * Removes items from inventory (triggering OnItemRemoved for quests), then grants Gold via GAS.
	 * @return True if the sale succeeded.
	 */
	bool TrySellItem(UInventoryComponent* PlayerInventory, UAbilitySystemComponent* PlayerASC,
	                 int32 SlotIndex, int32 Quantity);

	/** Broadcast after a successful purchase. Bind in UI for feedback. */
	UPROPERTY(BlueprintAssignable, Category = "Shop")
	FOnItemPurchased OnItemPurchased;

	/** Broadcast after a successful sale. Bind in UI for feedback. */
	UPROPERTY(BlueprintAssignable, Category = "Shop")
	FOnItemSold OnItemSold;

	UFUNCTION(BlueprintPure, Category = "Shop")
	UShopStockDataAsset* GetStockData() const { return StockData; }

	UFUNCTION(BlueprintPure, Category = "Shop")
	FText GetShopName() const { return ShopName; }

	UFUNCTION(BlueprintPure, Category = "Shop")
	float GetFallbackSellGoldPerUnit() const { return FallbackSellGoldPerUnit; }

	/** Returns current stock for an item. -1 means unlimited. 0 means sold out. */
	UFUNCTION(BlueprintPure, Category = "Shop")
	int32 GetCurrentStock(const UItemDataAsset* Item) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Stock configuration asset. Assign in the editor per shop type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TObjectPtr<UShopStockDataAsset> StockData;

	/** Blueprint widget class to open when the player interacts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TSubclassOf<UShopWidget> ShopWidgetClass;

	/** Display name shown in the interaction prompt and widget title. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FText ShopName;

	/**
	 * Gold granted per unit for items the player wants to sell that are not listed in StockData.
	 * Set to 0 to prevent selling unlisted items.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (ClampMin = "0.0"))
	float FallbackSellGoldPerUnit = 1.f;

private:
	void OpenShopWidgetForPlayer(AActor* Interactor);
	void CloseShopWidget();
	void InitializeStockQuantities();

	UFUNCTION()
	void OnShopWidgetClosed();

	UFUNCTION()
	void HandleNewDayAdvanced();

	const FShopStockEntry* FindStockEntry(const UItemDataAsset* Item) const;

	/**
	 * Runtime stock quantities keyed by item FName.
	 * -1 means unlimited; 0 means sold out.
	 * Only entries with DailyRestockQuantity != -1 are tracked.
	 */
	TMap<FName, int32> CurrentStockQuantities;

	UPROPERTY()
	TObjectPtr<UShopWidget> ShopWidgetInstance;

	TWeakObjectPtr<APlayerController> ShopOpenerController;
};
