#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ITooltipProvider.generated.h"

/**
 * Base interface for actors that can provide tooltip text.
 * Provides unified tooltip functionality for all interactable/farmable/harvestable actors.
 */
UINTERFACE(MinimalAPI)
class UTooltipProvider : public UInterface
{
	GENERATED_BODY()
};

class ITooltipProvider
{
	GENERATED_BODY()

public:
	/**
	 * Get the tooltip text to display for this actor.
	 * @return Text to display in the tooltip widget
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Tooltip")
	FText GetTooltipText() const;
};






