#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "USoilManagerSubsystem.generated.h"

class ASoilPlot;

UCLASS()
class FUNGIFIELDS_API USoilManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Soil Manager")
	void RegisterSoilPlot(ASoilPlot* Plot);

	UFUNCTION(BlueprintCallable, Category = "Soil Manager")
	void UnregisterSoilPlot(ASoilPlot* Plot);

	UFUNCTION(BlueprintPure, Category = "Soil Manager")
	int32 GetRegisteredPlotCount() const { return RegisteredPlots.Num(); }

	const TSet<TObjectPtr<ASoilPlot>>& GetRegisteredPlots() const { return RegisteredPlots; }

private:
	UPROPERTY()
	TSet<TObjectPtr<ASoilPlot>> RegisteredPlots;
};
