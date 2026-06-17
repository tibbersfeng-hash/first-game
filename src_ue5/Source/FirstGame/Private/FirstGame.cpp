// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"

#define LOCTEXT_NAMESPACE "FFirstGameModule"

void FFirstGameModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FirstGame Module Starting Up - 格斗萌主"));
}

void FFirstGameModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("FirstGame Module Shutting Down"));
}

IMPLEMENT_MODULE(FFirstGameModule, FirstGame)

#undef LOCTEXT_NAMESPACE
