#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UMilestoneDataAsset.generated.h"

/**
 * Defines a single trackable milestone with named tiers.
 * MilestoneID is the FName key used to report events via UMilestoneSubsystem::ReportEvent.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API UMilestoneDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Milestone")
	FName MilestoneID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Milestone")
	FText DisplayName;

	/** Ascending thresholds at which a new tier is reached. E.g. {10, 25, 50}. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Milestone")
	TArray<int32> TierThresholds;

	/** Display text for each tier, aligned by index to TierThresholds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Milestone")
	TArray<FText> TierDescriptions;
};
