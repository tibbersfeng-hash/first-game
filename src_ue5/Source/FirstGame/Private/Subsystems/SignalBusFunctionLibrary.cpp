// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Subsystems/SignalBusFunctionLibrary.h"
#include "FirstGame.h"
#include "Subsystems/SignalBusSubsystem.h"
#include "Engine/GameInstance.h"

USignalBusSubsystem* USignalBusFunctionLibrary::GetSignalBus(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	if (UGameInstance* GI = World->GetGameInstance())
	{
		return GI->GetSubsystem<USignalBusSubsystem>();
	}
	return nullptr;
}
