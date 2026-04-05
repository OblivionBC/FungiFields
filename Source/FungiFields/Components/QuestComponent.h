#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FungiFields/Data/Quest.h"
#include "QuestComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestsUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestCompleted, UQuest*, QuestDef, FQuestProgress, Progress);

class UCropDataAsset;
class USeedDataAsset;
class UItemDataAsset;
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
	UQuest* AddQuest(UQuest* Quest);

	UFUNCTION(BlueprintCallable)
	TArray<UQuest*> GetAllQuestDefinitions() const;

	void ForEachQuestDefinition(TFunctionRef<void(const UQuest*)> Callback) const;

	UFUNCTION(BlueprintCallable)
	bool GetQuestProgressForID(FName QuestID, FQuestProgress& OutProgress) const;

	UFUNCTION(BlueprintCallable)
	bool RemoveQuest(FName QuestID);

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestsUpdated OnQuestsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestCompleted OnQuestCompleted;

	UFUNCTION(BlueprintCallable, Category = "Quest Events")
	void SubscribeToComponentEvents();

	UFUNCTION(BlueprintCallable, Category = "Quest Events")
	void UnsubscribeFromComponentEvents();

private:
	UFUNCTION()
	void OnCropHarvested(AActor* Harvester, UCropDataAsset* CropData, int32 Quantity);

	UFUNCTION()
	void OnCropFullyGrown(AActor* Crop);

	UFUNCTION()
	void OnCropWithered(AActor* Crop);

	UFUNCTION()
	void OnSeedPlanted(AActor* Planter, USeedDataAsset* SeedData);

	UFUNCTION()
	void OnSoilTilled(AActor* Tiller);

	UFUNCTION()
	void OnSoilWatered(AActor* Waterer, AActor* SoilPlot);

	UFUNCTION()
	void OnItemAdded(UItemDataAsset* Item, int32 Amount, int32 NewTotal);

	UFUNCTION()
	void OnItemRemoved(UItemDataAsset* Item, int32 Amount, int32 NewTotal);

	UFUNCTION()
	void OnFarmingActionPerformed(AActor* Performer);

	bool AdvanceMatchingQuests(TFunction<bool(const UQuest*)> ShouldAdvance, int32 Amount);
	void TryAdvanceAndNotify(TFunction<bool(const UQuest*)> ShouldAdvance, int32 Amount);
	void DispatchQuestRewards(const UQuest* QuestDef, const FQuestProgress& Progress);

	UPROPERTY()
	TMap<FName, TObjectPtr<UQuest>> QuestDefinitions;

	UPROPERTY()
	TMap<FName, FQuestProgress> QuestProgress;
};
