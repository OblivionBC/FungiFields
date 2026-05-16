#include "FungiFieldsGameMode.h"
#include "Characters/FungiFieldsCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Subsystems/UDayNightSubsystem.h"
#include "Engine/Engine.h"

AFungiFieldsGameMode::AFungiFieldsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AFungiFieldsGameMode::FF_PrintTime()
{
	UDayNightSubsystem* DayNight = GetWorld()->GetSubsystem<UDayNightSubsystem>();
	if (!DayNight)
		return;

	const float Time = DayNight->GetTimeOfDay();
	const int32 Hour = FMath::FloorToInt(Time);
	const int32 Minute = FMath::FloorToInt((Time - Hour) * 60.0f);
	const FString Msg = FString::Printf(TEXT("Day %d  |  %02d:%02d  (%s)"),
		DayNight->GetCurrentDay(), Hour, Minute,
		DayNight->IsNight() ? TEXT("Night") : TEXT("Day"));

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, Msg);
	UE_LOG(LogTemp, Log, TEXT("FF_PrintTime: %s"), *Msg);
}

void AFungiFieldsGameMode::FF_SetTime(float Hour)
{
	UDayNightSubsystem* DayNight = GetWorld()->GetSubsystem<UDayNightSubsystem>();
	if (DayNight)
	{
		DayNight->SetTimeOfDay(Hour);
	}
}

void AFungiFieldsGameMode::FF_Morning()
{
	UDayNightSubsystem* DayNight = GetWorld()->GetSubsystem<UDayNightSubsystem>();
	if (DayNight)
	{
		DayNight->SkipToMorning();
	}
}

void AFungiFieldsGameMode::FF_Night()
{
	UDayNightSubsystem* DayNight = GetWorld()->GetSubsystem<UDayNightSubsystem>();
	if (DayNight)
	{
		DayNight->SkipToNight();
	}
}
