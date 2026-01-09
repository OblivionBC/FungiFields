#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "USoilContainerDataAsset.generated.h"

/**
 * Data Asset for defining soil container properties.
 * Contains information about the container mesh and container-specific settings.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API USoilContainerDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	USoilContainerDataAsset();

	/** Name of the container type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Properties")
	FName ContainerName;

	/** Static mesh for the container */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Visuals")
	TObjectPtr<UStaticMesh> ContainerMesh;
};


