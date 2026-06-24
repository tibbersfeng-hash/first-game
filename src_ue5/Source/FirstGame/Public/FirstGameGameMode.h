// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Game Mode — 格斗萌主

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FirstGameGameMode.generated.h"

class ALevelBuilder;

UCLASS()
class FIRSTGAME_API AFirstGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFirstGameGameMode();

protected:
	virtual void BeginPlay() override;

	/** 初始化玩家角色数据 */
	void InitializePlayerCharacter();

	/** 重试初始化玩家 (Timer 回调) */
	UFUNCTION()
	void RetryInitializePlayer();

	/** 初始化重试计数 */
	int32 PlayerInitRetryCount = 0;
	static constexpr int32 MaxPlayerInitRetries = 10;

	/** 玩家角色 Class */
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<class APlayerCharacter> PlayerCharacterClass;

	/** 默认地牢房间数 */
	UPROPERTY(EditDefaultsOnly, Category = "Dungeon")
	int32 TotalRooms = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Dungeon")
	int32 CurrentRoomIndex = 0;

	/** LevelBuilder Class（若指定，则自动 Spawn 一个构建可玩关卡）*/
	UPROPERTY(EditDefaultsOnly, Category = "Level")
	TSubclassOf<ALevelBuilder> LevelBuilderClass;

	UPROPERTY()
	ALevelBuilder* ActiveLevelBuilder;
};
