// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "FirstGameGameMode.h"
#include "Characters/PlayerCharacter.h"
#include "Engine/World.h"

AFirstGameGameMode::AFirstGameGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
}

void AFirstGameGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("FirstGame GameMode started — 格斗萌主"));
}
