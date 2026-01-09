#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UPlacementComponent.generated.h"

class UCameraComponent;
class UItemDataAsset;
class USoilDataAsset;
class USoilContainerDataAsset;
class ASoilPlot;
class UMaterialInterface;
class UUserWidget;
struct FInputActionValue;
struct FHitResult;

// Forward declarations
class AActor;

// Delegate declarations for placement events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlaceablePlaced, AActor*, Placer, AActor*, PlacedActor, UItemDataAsset*, PlaceableItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlaceablePickedUp, AActor*, Picker, USoilDataAsset*, SoilData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContainerPickedUp, AActor*, Picker, USoilContainerDataAsset*, ContainerData);

/**
 * Component responsible for handling placement of any placeable items (e.g., soil plots, cosmetics, etc.).
 * Manages preview rendering, ground tracing, validation, and placement logic.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlacementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Enter placement mode with a placeable item.
	 * @param PlaceableItem The item data asset that is placeable
	 */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void EnterPlacementMode(UItemDataAsset* PlaceableItem);

	/**
	 * Exit placement mode and clean up preview.
	 */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void ExitPlacementMode();

	/**
	 * Check if currently in placement mode.
	 * @return True if in placement mode
	 */
	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsInPlacementMode() const { return bIsInPlacementMode; }

	/**
	 * Set the camera component to use for placement traces.
	 * Should be called from owner's BeginPlay after components are initialized.
	 * @param Camera The camera component to use for line traces
	 */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void SetCamera(UCameraComponent* Camera);

	/**
	 * Attempt to place a placeable item at the current preview location.
	 * @param Value Input action value (unused, but required for input binding)
	 */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void PlaceItem(const FInputActionValue& Value);

	/**
	 * Attempt to pick up a placeable item at the cursor location.
	 * @param Value Input action value (unused, but required for input binding)
	 */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void PickupItem(const FInputActionValue& Value);

	/**
	 * Adjust placement rotation (called by scroll wheel).
	 * @param Value Input action value (positive for scroll up, negative for scroll down)
	 */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void AdjustPlacementRotation(const FInputActionValue& Value);

	/** Delegate broadcast when a placeable item is placed */
	UPROPERTY(BlueprintAssignable, Category = "Placement Events")
	FOnPlaceablePlaced OnPlaceablePlaced;

	/** Delegate broadcast when a placeable item is picked up */
	UPROPERTY(BlueprintAssignable, Category = "Placement Events")
	FOnPlaceablePickedUp OnPlaceablePickedUp;

	/** Delegate broadcast when a container is picked up */
	UPROPERTY(BlueprintAssignable, Category = "Placement Events")
	FOnContainerPickedUp OnContainerPickedUp;

protected:
	/**
	 * Update the preview actor position and rotation based on ground trace.
	 * Called every frame in TickComponent when in placement mode.
	 */
	void UpdatePreview();

	/**
	 * Perform ground trace from camera to find placement location.
	 * @param OutHit Hit result with surface normal
	 * @return True if trace hit a valid surface
	 */
	bool PerformGroundTrace(FHitResult& OutHit) const;

	/**
	 * Calculate rotation from surface normal.
	 * @param Normal Surface normal vector
	 * @return Rotation aligned to the surface
	 */
	FRotator CalculateRotationFromNormal(const FVector& Normal) const;

	/**
	 * Check if a location is valid for placement.
	 * @param Location World location to check
	 * @param Normal Surface normal at location
	 * @return True if placement is valid
	 */
	bool CanPlaceAtLocation(const FVector& Location, const FVector& Normal) const;

	/**
	 * Place a placeable item at the specified location with rotation.
	 * @param Location World location to place at
	 * @param Rotation Rotation aligned to ground surface
	 */
	void PlaceItemAtLocation(const FVector& Location, const FRotator& Rotation);

	/**
	 * Create or update the preview actor.
	 */
	void UpdatePreviewActor();

	/**
	 * Destroy the preview actor.
	 */
	void DestroyPreviewActor();

	/**
	 * Show the placement instruction widget.
	 */
	void ShowPlacementInstructions();

	/**
	 * Hide the placement instruction widget.
	 */
	void HidePlacementInstructions();

	/**
	 * Calculate the offset from the actor's root to the bottom of its bounding box.
	 * This ensures the bottom of the actor aligns with the ground when placed.
	 * @param Actor The actor to calculate bounds for
	 * @return The Z offset from actor root to bottom of bounding box (negative value)
	 */
	float CalculateActorBottomOffset(AActor* Actor) const;

	/** Widget class to display placement instructions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Settings")
	TSubclassOf<UUserWidget> PlacementInstructionWidgetClass;

	/** Class to use for preview actor (defaults to ASoilPlot for backward compatibility) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Settings")
	TSubclassOf<AActor> PreviewActorClass;

	/** Material to apply when placement is valid (green tint) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Settings")
	TObjectPtr<UMaterialInterface> PreviewMaterialValid;

	/** Material to apply when placement is invalid (red tint) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Settings")
	TObjectPtr<UMaterialInterface> PreviewMaterialInvalid;

	/** Maximum distance for ground trace */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Settings", meta = (ClampMin = "0.0"))
	float GroundTraceDistance = 1000.0f;

	/** Minimum distance from player for placement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Settings", meta = (ClampMin = "0.0"))
	float MinPlacementDistance = 50.0f;

	/** Radius for collision check when validating placement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Settings", meta = (ClampMin = "0.0"))
	float PlacementCheckRadius = 50.0f;

	/** Rotation adjustment step size (degrees per scroll) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement Settings", meta = (ClampMin = "1.0"))
	float RotationAdjustmentStep = 15.0f;

private:
	/** Camera component for line traces */
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;

	/** Whether currently in placement mode */
	UPROPERTY(VisibleAnywhere, Category = "Placement Data")
	bool bIsInPlacementMode = false;

	/** Currently equipped placeable item */
	UPROPERTY(VisibleAnywhere, Category = "Placement Data")
	TObjectPtr<UItemDataAsset> CurrentPlaceableItem = nullptr;

	/** Preview actor instance */
	UPROPERTY()
	TObjectPtr<AActor> PreviewActor = nullptr;

	/** Instance of the placement instruction widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> PlacementInstructionWidget = nullptr;

	/** Current preview location (actor root location) */
	UPROPERTY(VisibleAnywhere, Category = "Placement Data")
	FVector PreviewLocation = FVector::ZeroVector;

	/** Target bottom location where the bottom of the actor should align */
	UPROPERTY(VisibleAnywhere, Category = "Placement Data")
	FVector TargetBottomLocation = FVector::ZeroVector;

	/** Current preview rotation */
	UPROPERTY(VisibleAnywhere, Category = "Placement Data")
	FRotator PreviewRotation = FRotator::ZeroRotator;

	/** Whether current preview location is valid */
	UPROPERTY(VisibleAnywhere, Category = "Placement Data")
	bool bPreviewLocationValid = false;

	/** Current rotation offset (adjusted by scroll wheel) */
	UPROPERTY(VisibleAnywhere, Category = "Placement Data")
	float CurrentRotationOffset = 0.0f;

	/** Cached bottom offset for the current preview actor (reused for placement) */
	UPROPERTY(VisibleAnywhere, Category = "Placement Data")
	float CachedBottomOffset = 0.0f;

	/** Whether the bottom offset has been calculated and cached */
	UPROPERTY(VisibleAnywhere, Category = "Placement Data")
	bool bBottomOffsetCached = false;
};



