#pragma once

#include "CoreMinimal.h"
#include "UItemDataAsset.h"
#include "../ENUM/EToolType.h"
#include "../Interfaces/IUsable.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "UToolDataAsset.generated.h"

/**
 * Data Asset for defining tool properties.
 * Extends UItemDataAsset since tools are items that can be equipped.
 */
UCLASS(BlueprintType)
class FUNGIFIELDS_API UToolDataAsset : public UItemDataAsset, public IUsable
{
	GENERATED_BODY()

public:
	UToolDataAsset();

	// IUsable
	virtual bool UseItem_Implementation(AActor* User) override;
	virtual bool CanUseItem_Implementation(const AActor* User) const override;
	virtual bool ShouldConsumeOnUse_Implementation() const override { return false; }

	/** Type of tool */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties")
	EToolType ToolType = EToolType::Hoe;

	/** Effectiveness/power of the tool */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties", meta = (ClampMin = "0.0"))
	float ToolPower = 1.0f;

	/** Stamina required per use */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties", meta = (ClampMin = "0.0"))
	float StaminaCost = 10.0f;

	/** Interaction range for the tool */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties", meta = (ClampMin = "0.0"))
	float ToolRange = 200.0f;

	/** Particle effect to spawn when using this tool (Niagara) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Visuals")
	TObjectPtr<UNiagaraSystem> ToolParticleEffect;

	/** Particle effect to spawn when using this tool (Cascade - legacy) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Visuals")
	TObjectPtr<UParticleSystem> ToolParticleEffectCascade;
};