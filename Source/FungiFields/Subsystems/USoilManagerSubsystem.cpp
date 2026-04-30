#include "USoilManagerSubsystem.h"
#include "../Actors/ASoilPlot.h"

void USoilManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USoilManagerSubsystem::Deinitialize()
{
	RegisteredPlots.Empty();
	Super::Deinitialize();
}

void USoilManagerSubsystem::RegisterSoilPlot(ASoilPlot* Plot)
{
	if (IsValid(Plot))
	{
		RegisteredPlots.Add(Plot);
	}
}

void USoilManagerSubsystem::UnregisterSoilPlot(ASoilPlot* Plot)
{
	RegisteredPlots.Remove(Plot);
}
