// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// ADR-002: Autoload Singletons — 游戏管理器

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Menu UMETA(DisplayName = "Main Menu"),
	Playing UMETA(DisplayName = "Playing"),
	Paused UMETA(DisplayName = "Paused"),
	Cutscene UMETA(DisplayName = "Cutscene"),
	GameOver UMETA(DisplayName = "Game Over"),
	Victory UMETA(DisplayName = "Victory")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, EGameState, NewState);

UCLASS()
class FIRSTGAME_API UGameManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	EGameState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetGameState(EGameState NewState);

	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGameStateChanged OnGameStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	bool IsPlaying() const { return CurrentState == EGameState::Playing; }

	UFUNCTION(BlueprintCallable, Category = "Game State")
	bool IsPaused() const { return CurrentState == EGameState::Paused; }

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void ResumeGame();

private:
	UPROPERTY()
	EGameState CurrentState = EGameState::Menu;
};
