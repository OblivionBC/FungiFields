#include "UToolDataAsset.h"
#include "../Components/UFarmingComponent.h"

UToolDataAsset::UToolDataAsset()
{
	ToolType = EToolType::Hoe;
	ToolPower = 1.0f;
	StaminaCost = 10.0f;
	ToolRange = 200.0f;
}

bool UToolDataAsset::UseItem_Implementation(AActor* User)
{
	if (!User) return false;
	UFarmingComponent* FarmingComp = User->FindComponentByClass<UFarmingComponent>();
	if (!FarmingComp) return false;
	return FarmingComp->ExecuteUse();
}

bool UToolDataAsset::CanUseItem_Implementation(const AActor* User) const
{
	if (!User) return false;
	UFarmingComponent* FarmingComp = User->FindComponentByClass<UFarmingComponent>();
	return FarmingComp && FarmingComp->HasValidToolEquipped();
}