#include "ChestInventoryComponent.h"

UChestInventoryComponent::UChestInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bEnableReplication = true;
	SetInitialSlotCount(18);
	SetSupportsEquipping(false);
}

void UChestInventoryComponent::PostInitProperties()
{
	if (bEnableReplication)
	{
		SetIsReplicatedByDefault(true);
	}
	
	Super::PostInitProperties();
}


