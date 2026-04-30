#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "USporeJournalDataAsset.generated.h"

class UMilestoneDataAsset;

/**
 * Aggregates all milestone definitions for the Spore Journal.
 * Assign this asset to USporeJournalWidget::JournalData in a Blueprint default.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API USporeJournalDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spore Journal")
	TArray<TObjectPtr<UMilestoneDataAsset>> Milestones;
};
