#include "ChestInventoryComponent.h"

UChestInventoryComponent::UChestInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set replication flag to true by default for chests
	// This must be set in the constructor initialization list or here, before PostInitProperties runs
	bEnableReplication = true;
	
	// Configure for chest use
	SetInitialSlotCount(18);
	SetSupportsEquipping(false);
}

void UChestInventoryComponent::PostInitProperties()
{
	// SetIsReplicatedByDefault must be called during construction.
	// PostInitProperties is called during construction, so we can safely set it here.
	// Since bEnableReplication is set to true in the constructor, it will be true when PostInitProperties runs.
	if (bEnableReplication)
	{
		SetIsReplicatedByDefault(true);
	}
	
	// Call Super::PostInitProperties() which will also check bEnableReplication
	// but we've already set replication, so it won't set it again (it's idempotent)
	Super::PostInitProperties();
}

