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

class AActor;

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

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void EnterPlacementMode(UItemDataAsset* PlaceableItem);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void ExitPlacementMode();

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsInPlacementMode() const { return bIsInPlacementMode; }

	/** Should be called from owner's BeginPlay after components are initialized. */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void SetCamera(UCameraComponent* Camera);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void PlaceItem(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void PickupItem(const FInputActionValue& Value);

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
	void UpdatePreview();
	bool PerformGroundTrace(FHitResult& OutHit) const;
	FRotator CalculateRotationFromNormal(const FVector& Normal) const;
	bool CanPlaceAtLocation(const FVector& Location, const FVector& Normal) const;
	void PlaceItemAtLocation(const FVector& Location, const FRotator& Rotation);
	void UpdatePreviewActor();
	void DestroyPreviewActor();
	void ShowPlacementInstructions();
	void HidePlacementInstructions();

	/** @return The Z offset from actor root to bottom of bounding box (negative value) */
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
