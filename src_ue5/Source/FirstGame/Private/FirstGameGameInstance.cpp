// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGameGameInstance.h"
#include "FirstGame.h"
#include "Subsystems/SignalBusSubsystem.h"
#include "Subsystems/CombatDataSubsystem.h"
#include "Subsystems/GameManagerSubsystem.h"

void UFirstGameGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Log, TEXT("FirstGame GameInstance initialized — 格斗萌主"));

	// Subsystems are auto-registered via UCLASS() + UGameInstanceSubsystem
	// SignalBusSubsystem, CombatDataSubsystem, GameManagerSubsystem
}

void UFirstGameGameInstance::Shutdown()
{
	Super::Shutdown();
	UE_LOG(LogTemp, Log, TEXT("FirstGame GameInstance shutdown"));
}
