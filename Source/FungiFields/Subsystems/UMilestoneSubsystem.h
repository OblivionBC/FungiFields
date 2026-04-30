#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UMilestoneSubsystem.generated.h"

class USporeJournalDataAsset;
class UMilestoneDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMilestoneReached, FName, MilestoneID, int32, Tier);

/**
 * Tracks numerical milestones independent of the quest system.
 * Call ReportEvent with a milestone ID to increment its counter.
 * Fires OnMilestoneReached when a tier threshold is crossed.
 *
 * Known milestone IDs: VillagersRecruited, CropsHarvested, SeedsPlanted.
 * Register USporeJournalDataAsset via RegisterMilestoneData to enable tier tracking.
 */
UCLASS()
class FUNGIFIELDS_API UMilestoneSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Increment the counter for a milestone by the given amount.
	 * Fires OnMilestoneReached if a tier threshold is crossed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Milestones")
	void ReportEvent(FName MilestoneID, int32 Amount = 1);

	UFUNCTION(BlueprintPure, Category = "Milestones")
	int32 GetProgress(FName MilestoneID) const;

	/**
	 * Register milestone definitions so ReportEvent can check tier thresholds.
	 * Typically called from USporeJournalWidget::NativeConstruct.
	 */
	UFUNCTION(BlueprintCallable, Category = "Milestones")
	void RegisterMilestoneData(USporeJournalDataAsset* JournalData);

	UPROPERTY(BlueprintAssignable, Category = "Milestones")
	FOnMilestoneReached OnMilestoneReached;

private:
	UPROPERTY()
	TMap<FName, int32> MilestoneCounts;

	/** Milestone definitions keyed by MilestoneID for O(1) tier lookups. */
	UPROPERTY()
	TMap<FName, TObjectPtr<UMilestoneDataAsset>> MilestoneDefinitions;

	void CheckTierCrossing(FName MilestoneID, int32 OldCount, int32 NewCount);
};
