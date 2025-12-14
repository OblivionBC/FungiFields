#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UItemDataAsset.generated.h"

class USoilDataAsset;
class ASoilPlot;
class AActor;

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

	/** Image to display in Inventory and Hotbar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties")
	UTexture2D* ItemIcon;

	/** Mesh to display when item is equipped (attached to RightHandItemSlot socket) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties")
	UStaticMesh* ItemMesh;

	/** If true, this item can be placed in the world (e.g., soil plots) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement")
	bool bIsPlaceable = false;

	/** The soil type to place (only used if bIsPlaceable is true) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement", meta = (EditCondition = "bIsPlaceable"))
	TObjectPtr<USoilDataAsset> PlaceableSoilDataAsset;

	/** Actor class to spawn when placing (defaults to ASoilPlot for soil items, but can be any actor for cosmetics, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement", meta = (EditCondition = "bIsPlaceable"))
	TSubclassOf<AActor> PlaceableActorClass;

	/** Maximum distance for placement trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement", meta = (EditCondition = "bIsPlaceable", ClampMin = "0.0"))
	float PlacementTraceDistance = 1000.0f;

	/** Vertical offset for preview placement */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement", meta = (EditCondition = "bIsPlaceable"))
	float PreviewOffsetZ = 0.0f;
};

