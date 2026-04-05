#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../ENUM/EToolType.h"
#include "../ENUM/EFarmerRole.h"
#include "FarmerVillagerCharacter.generated.h"

class UFarmingComponent;
class UInventoryComponent;
class UFarmerTargetingComponent;

/**
 * Minimal autonomous villager used for Phase 1 farming AI vertical slice.
 * Owns reusable farming components and exposes role configuration to the controller.
 */
UCLASS()
class FUNGIFIELDS_API AFarmerVillagerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFarmerVillagerCharacter();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	UFarmingComponent* GetFarmingComponent() const { return FarmingComponent; }

	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	UFarmerTargetingComponent* GetTargetingComponent() const { return TargetingComponent; }

	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	EFarmerRole GetAssignedRole() const { return AssignedRole; }

	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	EToolType GetRoleToolType() const { return ResolveRoleToolType(AssignedRole); }

	UFUNCTION(BlueprintPure, Category = "AI|Farming")
	float GetRoleToolPower() const { return RoleToolPower; }

	UFUNCTION(BlueprintPure, Category = "AI|Debug")
	bool IsAIDebugLoggingEnabled() const { return bEnableAIDebugLogs; }

protected:
	/** Shared farming executor used by both player and AI flows. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFarmingComponent> FarmingComponent;

	/** Optional inventory component for compatibility with existing shared systems. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	/** Role-agnostic target finder used by AI controllers. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFarmerTargetingComponent> TargetingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Role")
	EFarmerRole AssignedRole = EFarmerRole::Harvester;

	/** Tool power passed into UFarmingComponent::ExecuteFarmingAction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Role", meta = (ClampMin = "0.1"))
	float RoleToolPower = 1.0f;

	/** Toggle-friendly debug logs for quick PIE iteration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Debug")
	bool bEnableAIDebugLogs = true;

private:
	EToolType ResolveRoleToolType(EFarmerRole FarmerRole) const;
};
