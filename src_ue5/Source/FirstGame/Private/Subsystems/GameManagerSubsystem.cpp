// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Subsystems/GameManagerSubsystem.h"
#include "FirstGame.h"

void UGameManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("GameManagerSubsystem initialized"));
}

void UGameManagerSubsystem::SetGameState(EGameState NewState)
{
	if (CurrentState != NewState)
	{
		EGameState OldState = CurrentState;
		CurrentState = NewState;
		UE_LOG(LogTemp, Log, TEXT("Game State: %d -> %d"), (int)OldState, (int)NewState);
		OnGameStateChanged.Broadcast(NewState);
	}
}

void UGameManagerSubsystem::PauseGame()
{
	SetGameState(EGameState::Paused);
	if (UWorld* World = GetWorld())
	{
		World->GetFirstPlayerController()->SetPause(true);
	}
}

void UGameManagerSubsystem::ResumeGame()
{
	SetGameState(EGameState::Playing);
	if (UWorld* World = GetWorld())
	{
		World->GetFirstPlayerController()->SetPause(false);
	}
}
