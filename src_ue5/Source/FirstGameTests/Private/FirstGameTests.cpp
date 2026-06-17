// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGameTests.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

void FFirstGameTestsModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FirstGameTests Module Started"));
}

void FFirstGameTestsModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("FirstGameTests Module Stopped"));
}

IMPLEMENT_MODULE(FFirstGameTestsModule, FirstGameTests)
