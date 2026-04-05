#include "FarmerTargetingComponent.h"
#include "Engine/World.h"
#include "../../Interfaces/IHarvestableInterface.h"
#include "../../Components/UFarmingComponent.h"
#include "../../Components/UCropGrowthComponent.h"
#include "../../Subsystems/UCropManagerSubsystem.h"
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

AActor* UFarmerTargetingComponent::FindBestPlantTarget(const FVector& Origin)
{
	// TODO: HH Implement planter role target acquisition in Phase 2.
	return nullptr;
}

AActor* UFarmerTargetingComponent::FindBestWaterTarget(const FVector& Origin)
{
	// TODO: HH Implement waterer role target acquisition in Phase 2.
	return nullptr;
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
