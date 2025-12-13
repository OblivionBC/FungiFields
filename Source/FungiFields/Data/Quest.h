#pragma once

#include "CoreMinimal.h"
#include "FungiFields/ENUM/QuestState.h"
#include "FungiFields/ENUM/EQuestEventType.h"
#include "UObject/Object.h"
#include "Quest.generated.h"

class UItemDataAsset;
class UCropDataAsset;
class USeedDataAsset;

UCLASS()
class FUNGIFIELDS_API UQuest : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	FName QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
	int32 RequiredProgress = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Quest")
	EQuestState State = EQuestState::NotStarted;

	/** Type of event this quest listens to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest Requirements")
	EQuestEventType QuestEventType = EQuestEventType::None;

	/** Optional: Specific item this quest requires (for ItemAdded/ItemRemoved events) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest Requirements", meta = (EditCondition = "QuestEventType == EQuestEventType::ItemAdded || QuestEventType == EQuestEventType::ItemRemoved"))
	TSoftObjectPtr<UItemDataAsset> RequiredItem;

	/** Optional: Specific crop this quest requires (for CropHarvested events) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest Requirements", meta = (EditCondition = "QuestEventType == EQuestEventType::CropHarvested"))
	TSoftObjectPtr<UCropDataAsset> RequiredCrop;

	/** Optional: Specific seed this quest requires (for SeedPlanted events) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest Requirements", meta = (EditCondition = "QuestEventType == EQuestEventType::SeedPlanted"))
	TSoftObjectPtr<USeedDataAsset> RequiredSeed;

	/**
	 * Check if this quest should respond to an item added event.
	 * @param Item The item that was added
	 * @param Quantity The quantity added
	 * @return True if this quest should update progress
	 */
	UFUNCTION(BlueprintCallable, Category="Quest")
	bool ShouldRespondToItemAdded(UItemDataAsset* Item, int32 Quantity) const;

	/**
	 * Check if this quest should respond to a crop harvested event.
	 * @param CropData The crop that was harvested
	 * @param Quantity The quantity harvested
	 * @return True if this quest should update progress
	 */
	UFUNCTION(BlueprintCallable, Category="Quest")
	bool ShouldRespondToCropHarvested(UCropDataAsset* CropData, int32 Quantity) const;

	/**
	 * Check if this quest should respond to a seed planted event.
	 * @param SeedData The seed that was planted
	 * @return True if this quest should update progress
	 */
	UFUNCTION(BlueprintCallable, Category="Quest")
	bool ShouldRespondToSeedPlanted(USeedDataAsset* SeedData) const;

	UFUNCTION(BlueprintCallable, Category="Quest")
	void StartQuest();

	UFUNCTION(BlueprintCallable, Category="Quest")
	void AddProgress(int32 Amount);

	UFUNCTION(BlueprintCallable, Category="Quest")
	void FailQuest();
};
