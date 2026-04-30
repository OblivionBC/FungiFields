#pragma once

#include "CoreMinimal.h"
#include "FungiFields/ENUM/QuestState.h"
#include "FungiFields/ENUM/EQuestEventType.h"
#include "Engine/DataAsset.h"
#include "Quest.generated.h"

class UItemDataAsset;
class UCropDataAsset;
class USeedDataAsset;

USTRUCT(BlueprintType)
struct FUNGIFIELDS_API FQuestProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FName QuestID;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	int32 CurrentProgress = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	EQuestState State = EQuestState::NotStarted;
};

UCLASS()
class FUNGIFIELDS_API UQuest : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	int32 RequiredProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Requirements")
	EQuestEventType QuestEventType = EQuestEventType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Requirements", meta = (EditCondition = "QuestEventType == EQuestEventType::ItemAdded || QuestEventType == EQuestEventType::ItemRemoved"))
	TObjectPtr<UItemDataAsset> RequiredItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Requirements", meta = (EditCondition = "QuestEventType == EQuestEventType::CropHarvested || QuestEventType == EQuestEventType::CropFullyGrown || QuestEventType == EQuestEventType::CropWithered"))
	TObjectPtr<UCropDataAsset> RequiredCrop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Requirements", meta = (EditCondition = "QuestEventType == EQuestEventType::SeedPlanted"))
	TObjectPtr<USeedDataAsset> RequiredSeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	float GoldReward = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	float XPReward = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	TObjectPtr<UItemDataAsset> RewardItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	int32 RewardItemQuantity = 1;

	/** Quest description shown in the journal UI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestDescription;

	/** Next quest in the chain. Automatically added to the player when this quest is collected. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Chain")
	TObjectPtr<UQuest> NextQuest;

	/** All of these quests must be Completed before this quest can be offered or auto-started. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Prerequisites")
	TArray<TObjectPtr<UQuest>> PrerequisiteQuests;

	bool ShouldRespondToItemAdded(UItemDataAsset* Item, int32 Quantity) const;
	bool ShouldRespondToItemRemoved(UItemDataAsset* Item, int32 Quantity) const;
	bool ShouldRespondToCropHarvested(UCropDataAsset* CropData, int32 Quantity) const;
	bool ShouldRespondToCropFullyGrown(UCropDataAsset* CropData) const;
	bool ShouldRespondToCropWithered(UCropDataAsset* CropData) const;
	bool ShouldRespondToSeedPlanted(USeedDataAsset* SeedData) const;

	void StartQuest(FQuestProgress& Progress) const;
	void AddProgress(FQuestProgress& Progress, int32 Amount) const;
	void FailQuest(FQuestProgress& Progress) const;
};
