#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IUsable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UUsable : public UInterface
{
	GENERATED_BODY()
};

class FUNGIFIELDS_API IUsable
{
	GENERATED_BODY()
public:
	/** Attempt to use this item. Return true if the use succeeded. */
	UFUNCTION(BlueprintNativeEvent, Category = "Item Use")
	bool UseItem(AActor* User);

	/** Return true if the item can currently be used by User. */
	UFUNCTION(BlueprintNativeEvent, Category = "Item Use")
	bool CanUseItem(const AActor* User) const;

	/** Return true if one unit of this item should be removed from inventory after a successful use. */
	UFUNCTION(BlueprintNativeEvent, Category = "Item Use")
	bool ShouldConsumeOnUse() const;
};
