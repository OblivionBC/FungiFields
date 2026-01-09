#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
class UQuest;
class UCropDataAsset;
class USeedDataAsset;
class UItemDataAsset;
#include "QuestComponent.generated.h"

// Forward declarations
class AActor;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable)
	UQuest* AddQuest(UQuest* QuestClass);

	UFUNCTION(BlueprintCallable)
	TArray<UQuest*> GetAllQuests() const;

	UFUNCTION(BlueprintCallable)
	bool RemoveQuest(FName QuestID);

	/**
	 * Subscribe to component events from owner's components.
	 * Called automatically in BeginPlay.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Events")
	void SubscribeToComponentEvents();

	/**
	 * Unsubscribe from component events.
	 * Called automatically in EndPlay.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Events")
	void UnsubscribeFromComponentEvents();

private:
	/**
	 * Event handler for crop harvested events.
	 * Iterates through active quests and updates progress if applicable.
	 */
	UFUNCTION()
	void OnCropHarvested(AActor* Harvester, UCropDataAsset* CropData, int32 Quantity);

	/**
	 * Event handler for seed planted events.
	 * Iterates through active quests and updates progress if applicable.
	 */
	UFUNCTION()
	void OnSeedPlanted(AActor* Planter, USeedDataAsset* SeedData);

	/**
	 * Event handler for soil tilled events.
	 * Iterates through active quests and updates progress if applicable.
	 */
	UFUNCTION()
	void OnSoilTilled(AActor* Tiller);

	/**
	 * Event handler for soil watered events.
	 * Iterates through active quests and updates progress if applicable.
	 */
	UFUNCTION()
	void OnSoilWatered(AActor* Waterer, AActor* SoilPlot);

	/**
	 * Event handler for item added events from InventoryComponent.
	 * Iterates through active quests and updates progress if applicable.
	 */
	UFUNCTION()
	void OnItemAdded(UItemDataAsset* Item, int32 Amount, int32 NewTotal);

	UPROPERTY()
	TMap<FName, UQuest*> ActiveQuests;
};
