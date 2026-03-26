// Copyright Epic Games, Inc. All Rights Reserved.

#include "PluginMakingGameMode.h"
#include "PluginMakingCharacter.h"
#include "UObject/ConstructorHelpers.h"

APluginMakingGameMode::APluginMakingGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
