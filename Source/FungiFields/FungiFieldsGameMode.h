#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FungiFieldsGameMode.generated.h"

UCLASS(minimalapi)
class AFungiFieldsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFungiFieldsGameMode();

	// ── Debug console commands (type in ~ console) ────────────────────────────

	/** Print current in-game time and day to the screen. Usage: FF_PrintTime */
	UFUNCTION(Exec)
	void FF_PrintTime();

	/** Set the in-game time of day. Usage: FF_SetTime 14.5 */
	UFUNCTION(Exec)
	void FF_SetTime(float Hour);

	/** Jump to morning (DayStartHour) and advance the day counter. Usage: FF_Morning */
	UFUNCTION(Exec)
	void FF_Morning();

	/** Jump to night (NightStartHour). Usage: FF_Night */
	UFUNCTION(Exec)
	void FF_Night();
};



