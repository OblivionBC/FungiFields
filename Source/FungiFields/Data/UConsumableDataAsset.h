#pragma once

#include "CoreMinimal.h"
#include "UItemDataAsset.h"
#include "../Interfaces/IUsable.h"
#include "UConsumableDataAsset.generated.h"

/**
 * Data Asset for items consumed on use (food, potions).
 * Implements IUsable so the character's Left Click dispatcher routes through it.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API UConsumableDataAsset : public UItemDataAsset, public IUsable
{
	GENERATED_BODY()

public:
	UConsumableDataAsset();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "0.0"))
	float HealthRestoreAmount = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "0.0"))
	float StaminaRestoreAmount = 0.f;

	// IUsable
	virtual bool UseItem_Implementation(AActor* User) override;
	virtual bool CanUseItem_Implementation(const AActor* User) const override;
	virtual bool ShouldConsumeOnUse_Implementation() const override { return true; }
};
