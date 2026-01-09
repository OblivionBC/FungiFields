#include "FungiFieldsGameMode.h"
#include "Characters/FungiFieldsCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFungiFieldsGameMode::AFungiFieldsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
