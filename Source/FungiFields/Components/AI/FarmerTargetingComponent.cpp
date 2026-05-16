#include "FarmerTargetingComponent.h"
#include "Engine/World.h"
#include "../../Interfaces/IHarvestableInterface.h"
#include "../../Interfaces/IFarmableInterface.h"
#include "../../Components/UFarmingComponent.h"
#include "../../Components/UCropGrowthComponent.h"
#include "../../Subsystems/UCropManagerSubsystem.h"
#include "../../Subsystems/USoilManagerSubsystem.h"
#include "../../Actors/ASoilPlot.h"
#include "../../Actors/ACropBase.h"
#include "../../Data/USeedDataAsset.h"
#include "../../ENUM/EToolType.h"

UFarmerTargetingComponent::UFarmerTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AActor* UFarmerTargetingComponent::FindBestHarvestTarget(const FVector& Origin, const UFarmingComponent* FarmingComponent,
                                                          EToolType ToolType, const TArray<TWeakObjectPtr<ASoilPlot>>& AllowedPlots)
{
	UWorld* World = GetWorld();
	if (!World || !FarmingComponent)
		return nullptr;

	const float CurrentTimeSeconds = World->GetTimeSeconds();
	if (!ShouldRescanHarvestTargets(CurrentTimeSeconds))
		return IsValid(CachedHarvestTarget) ? CachedHarvestTarget.Get() : nullptr;

	LastHarvestScanTimestamp = CurrentTimeSeconds;

	UCropManagerSubsystem* CropManager = World->GetSubsystem<UCropManagerSubsystem>();
	if (!CropManager)
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const TObjectPtr<UCropGrowthComponent>& GrowthComp : CropManager->GetRegisteredCrops())
	{
		if (!IsValid(GrowthComp))
			continue;

		AActor* Candidate = GrowthComp->GetOwner();
		float DistanceSquared = TNumericLimits<float>::Max();
		if (!IsActorValidHarvestTarget(Candidate, Origin, FarmingComponent, ToolType, DistanceSquared))
			continue;

		if (!AllowedPlots.IsEmpty())
		{
			const ACropBase* Crop = Cast<ACropBase>(Candidate);
			const ASoilPlot* PlotOwner = Crop ? Crop->GetParentSoil() : nullptr;
			if (!PlotOwner || !AllowedPlots.ContainsByPredicate([PlotOwner](const TWeakObjectPtr<ASoilPlot>& W) {
				return W.Get() == PlotOwner;
			})) continue;
		}

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Candidate;
		}
	}

	CachedHarvestTarget = BestTarget;
	return BestTarget;
}

AActor* UFarmerTargetingComponent::FindBestPlantTarget(const FVector& Origin, USeedDataAsset* AvailableSeed,
                                                        const TArray<TWeakObjectPtr<ASoilPlot>>& AllowedPlots)
{
	UWorld* World = GetWorld();
	if (!World || !AvailableSeed)
		return nullptr;

	if (AllowedPlots.IsEmpty())
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<ASoilPlot>& WeakPlot : AllowedPlots)
	{
		ASoilPlot* Plot = WeakPlot.Get();
		if (!IsValid(Plot))
			continue;

		if (!IFarmableInterface::Execute_CanAcceptSeed(Plot))
			continue;

		const float DistanceSquared = FVector::DistSquared(Origin, Plot->GetActorLocation());
		if (DistanceSquared > FMath::Square(PlantScanRadius))
			continue;

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Plot;
		}
	}

	return BestTarget;
}

AActor* UFarmerTargetingComponent::FindBestWaterTarget(const FVector& Origin,
                                                        const TArray<TWeakObjectPtr<ASoilPlot>>& AllowedPlots)
{
	UWorld* World = GetWorld();
	if (!World)
		return nullptr;

	if (AllowedPlots.IsEmpty())
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<ASoilPlot>& WeakPlot : AllowedPlots)
	{
		ASoilPlot* Plot = WeakPlot.Get();
		if (!IsValid(Plot))
			continue;

		if (!IFarmableInterface::Execute_CanInteractWithTool(Plot, EToolType::WateringCan, nullptr))
			continue;

		const float DistanceSquared = FVector::DistSquared(Origin, Plot->GetActorLocation());
		if (DistanceSquared > FMath::Square(WaterScanRadius))
			continue;

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Plot;
		}
	}

	return BestTarget;
}

bool UFarmerTargetingComponent::ShouldRescanHarvestTargets(float CurrentTimeSeconds) const
{
	if (!IsValid(CachedHarvestTarget))
	{
		return true;
	}

	return (CurrentTimeSeconds - LastHarvestScanTimestamp) >= HarvestScanInterval;
}

bool UFarmerTargetingComponent::IsActorValidHarvestTarget(AActor* Candidate, const FVector& Origin, const UFarmingComponent* FarmingComponent, EToolType ToolType, float& OutDistanceSquared) const
{
	OutDistanceSquared = TNumericLimits<float>::Max();

	if (!IsValid(Candidate) || !Candidate->Implements<UHarvestableInterface>())
	{
		return false;
	}

	OutDistanceSquared = FVector::DistSquared(Origin, Candidate->GetActorLocation());
	if (OutDistanceSquared > FMath::Square(HarvestScanRadius))
	{
		return false;
	}

	return FarmingComponent->CanPerformFarmingAction(Candidate, ToolType, nullptr);
}
