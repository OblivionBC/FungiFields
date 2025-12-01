#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../ENUM/EToolType.h"
#include "UFarmingComponent.generated.h"

class UCameraComponent;
class UInventoryComponent;
class UCharacterAttributeSet;
class UToolDataAsset;
struct FInputActionValue;

/**
 * Component responsible for handling farming tool usage.
 * Performs line traces and interacts with farmable/harvestable actors via interfaces.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UFarmingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFarmingComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	/**
	 * Use the currently equipped tool.
	 * Performs line trace and interacts with target actor.
	 * @param Value Input action value (unused, but required for input binding)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UseEquippedTool(const FInputActionValue& Value);

	/**
	 * Set the equipped tool from inventory.
	 * Called when inventory changes or tool is equipped.
	 * @param ToolType Type of tool
	 * @param ToolPower Power/effectiveness of tool
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void SetEquippedTool(EToolType ToolType, float ToolPower);

	/**
	 * Get the currently equipped tool type.
	 * @return Tool type, or EToolType::Hoe if no tool equipped
	 */
	UFUNCTION(BlueprintPure, Category = "Farming")
	EToolType GetCurrentToolType() const { return CurrentToolType; }

	/**
	 * Get the currently equipped tool power.
	 * @return Tool power
	 */
	UFUNCTION(BlueprintPure, Category = "Farming")
	float GetCurrentToolPower() const { return CurrentToolPower; }

	/**
	 * Check if a valid tool is currently equipped.
	 * @return True if a tool is equipped, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Farming")
	bool HasValidToolEquipped() const { return bHasValidTool; }

	/**
	 * Set the camera component to use for tool traces.
	 * Should be called from owner's BeginPlay after components are initialized.
	 * @param Camera The camera component to use for line traces
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void SetCamera(UCameraComponent* Camera);

	/**
	 * Update equipped tool from inventory.
	 * Checks equipped slot and extracts tool data if it's a tool.
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UpdateEquippedTool();
	
protected:

	/**
	 * Perform line trace for tool interaction.
	 * @param OutHit Hit result if trace succeeds
	 * @return True if trace hit something
	 */
	bool PerformToolTrace(FHitResult& OutHit) const;

	/**
	 * Consume stamina for tool usage.
	 * @param StaminaCost Amount of stamina to consume
	 * @return True if stamina was available and consumed
	 */
	bool ConsumeStamina(float StaminaCost);

private:
	/** Camera component for line traces */
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;

	/** Currently equipped tool type */
	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	EToolType CurrentToolType = EToolType::Hoe;

	/** Currently equipped tool power */
	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	float CurrentToolPower = 1.0f;

	/** Whether a valid tool is currently equipped */
	UPROPERTY(VisibleAnywhere, Category = "Farming Data")
	bool bHasValidTool = false;

	/** Maximum distance for tool interaction */
	UPROPERTY(EditAnywhere, Category = "Farming Settings", meta = (ClampMin = "0.0"))
	float ToolTraceDistance = 800.0f;
};
