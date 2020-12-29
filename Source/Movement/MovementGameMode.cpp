// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "MovementGameMode.h"
#include "MovementCharacter.h"
#include "ClimbingCharacter.h"
#include "Servant.h"
#include "UObject/ConstructorHelpers.h"

AMovementGameMode::AMovementGameMode()
{

	DefaultPawnClass = AServant::StaticClass();
}
