#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UItemDataAsset.generated.h"

/**
 * Data Asset for defining immutable item properties.
 * Follows data-driven design principles - all item configuration is external to C++ code.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API UItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UItemDataAsset();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties")
	TSoftObjectPtr<UTexture2D> ItemIcon;
};

