// Copyright Epic Games, Inc. All Rights Reserved.

#include "LearnUE4GameMode.h"
#include "LearnUE4Character.h"
#include "UObject/ConstructorHelpers.h"

ALearnUE4GameMode::ALearnUE4GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
