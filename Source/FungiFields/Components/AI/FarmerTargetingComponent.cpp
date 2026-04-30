#include "FarmerTargetingComponent.h"
#include "Engine/World.h"
#include "../../Interfaces/IHarvestableInterface.h"
#include "../../Interfaces/IFarmableInterface.h"
#include "../../Components/UFarmingComponent.h"
#include "../../Components/UCropGrowthComponent.h"
#include "../../Subsystems/UCropManagerSubsystem.h"
#include "../../Subsystems/USoilManagerSubsystem.h"
#include "../../Actors/ASoilPlot.h"
#include "../../Data/USeedDataAsset.h"
#include "../../ENUM/EToolType.h"

UFarmerTargetingComponent::UFarmerTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AActor* UFarmerTargetingComponent::FindBestHarvestTarget(const FVector& Origin, const UFarmingComponent* FarmingComponent, EToolType ToolType)
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
		if (IsActorValidHarvestTarget(Candidate, Origin, FarmingComponent, ToolType, DistanceSquared) && DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Candidate;
		}
	}

	CachedHarvestTarget = BestTarget;
	return BestTarget;
}

AActor* UFarmerTargetingComponent::FindBestPlantTarget(const FVector& Origin, USeedDataAsset* AvailableSeed)
{
	UWorld* World = GetWorld();
	if (!World || !AvailableSeed)
		return nullptr;

	USoilManagerSubsystem* SoilManager = World->GetSubsystem<USoilManagerSubsystem>();
	if (!SoilManager)
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const TObjectPtr<ASoilPlot>& Plot : SoilManager->GetRegisteredPlots())
	{
		if (!IsValid(Plot))
			continue;

		if (!Plot->Implements<UFarmableInterface>())
			continue;

		if (!IFarmableInterface::Execute_CanAcceptSeed(Plot.Get()))
			continue;

		const float DistanceSquared = FVector::DistSquared(Origin, Plot->GetActorLocation());
		if (DistanceSquared > FMath::Square(PlantScanRadius))
			continue;

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Plot.Get();
		}
	}

	return BestTarget;
}

AActor* UFarmerTargetingComponent::FindBestWaterTarget(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World)
		return nullptr;

	USoilManagerSubsystem* SoilManager = World->GetSubsystem<USoilManagerSubsystem>();
	if (!SoilManager)
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const TObjectPtr<ASoilPlot>& Plot : SoilManager->GetRegisteredPlots())
	{
		if (!IsValid(Plot))
			continue;

		if (!Plot->Implements<UFarmableInterface>())
			continue;

		if (!IFarmableInterface::Execute_CanInteractWithTool(Plot.Get(), EToolType::WateringCan, nullptr))
			continue;

		const float DistanceSquared = FVector::DistSquared(Origin, Plot->GetActorLocation());
		if (DistanceSquared > FMath::Square(WaterScanRadius))
			continue;

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Plot.Get();
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
