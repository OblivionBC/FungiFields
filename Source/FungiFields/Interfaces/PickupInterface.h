#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPickupInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for objects that can be picked up by the player.
 */
class FUNGIFIELDS_API IPickupInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PickupObject();
};
