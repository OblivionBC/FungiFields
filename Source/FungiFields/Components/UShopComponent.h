#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Data/UShopStockDataAsset.h"
#include "UShopComponent.generated.h"

class UInventoryComponent;
class UAbilitySystemComponent;
class UShopWidget;
struct FShopStockEntry;
class UItemDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemPurchased, UItemDataAsset*, Item, int32, Quantity, float, TotalCost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemSold,      UItemDataAsset*, Item, int32, Quantity, float, TotalProceeds);

/**
 * Drop-in shop component. Attach to any actor (villager, market stall, chest) to grant
 * buy/sell behaviour. UBT_DialogueComponent on the same actor automatically surfaces a
 * Shop button in the dialogue menu — no manual linking required.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShopComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Opens the shop widget for the given interactor pawn. Called by UBT_DialogueWidget. */
	void OpenShopWidget(AActor* Interactor);

	/** Closes the active shop widget and restores input mode. */
	void CloseShopWidget();

	bool TryBuyItem(UInventoryComponent* PlayerInventory, UAbilitySystemComponent* PlayerASC,
	                const FShopStockEntry& Entry, int32 Quantity);

	bool TrySellItem(UInventoryComponent* PlayerInventory, UAbilitySystemComponent* PlayerASC,
	                 int32 SlotIndex, int32 Quantity);

	/** Returns -1 (unlimited), 0 (sold out), or current available quantity. */
	UFUNCTION(BlueprintPure, Category = "Shop")
	int32 GetCurrentStock(const UItemDataAsset* Item) const;

	UFUNCTION(BlueprintPure, Category = "Shop") UShopStockDataAsset* GetStockData()             const { return StockData; }
	UFUNCTION(BlueprintPure, Category = "Shop") FText                GetShopName()               const { return ShopName; }
	UFUNCTION(BlueprintPure, Category = "Shop") float                GetFallbackSellGoldPerUnit() const { return FallbackSellGoldPerUnit; }

	UPROPERTY(BlueprintAssignable, Category = "Shop") FOnItemPurchased OnItemPurchased;
	UPROPERTY(BlueprintAssignable, Category = "Shop") FOnItemSold      OnItemSold;

	// ── Editor config ──────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TObjectPtr<UShopStockDataAsset> StockData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TSubclassOf<UShopWidget> ShopWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FText ShopName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (ClampMin = "0.0"))
	float FallbackSellGoldPerUnit = 1.f;

	/** Starting gold pool. Player can sell to this merchant only while it has enough gold. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop|Economy", meta = (ClampMin = "0.0"))
	float MerchantGold = 500.f;

	/** If true, the merchant has unlimited gold (e.g. a player-run market stall). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Economy")
	bool bHasInfiniteGold = false;

	UFUNCTION(BlueprintPure, Category = "Shop")
	float GetMerchantGold() const { return MerchantGold; }

	UFUNCTION(BlueprintPure, Category = "Shop")
	bool HasEnoughGoldToBuy(float Amount) const { return bHasInfiniteGold || MerchantGold >= Amount; }

private:
	void InitializeStockQuantities();
	const FShopStockEntry* FindStockEntry(const UItemDataAsset* Item) const;

	UFUNCTION() void HandleNewDayAdvanced();
	UFUNCTION() void OnShopWidgetClosed();

	TMap<FName, int32> CurrentStockQuantities;

	UPROPERTY() TObjectPtr<UShopWidget>       ShopWidgetInstance;
	TWeakObjectPtr<APlayerController>          ShopOpenerController;
};
