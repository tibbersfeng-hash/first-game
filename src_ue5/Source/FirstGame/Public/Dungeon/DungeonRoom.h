// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Dungeon Room — single combat arena (now with WaveManager integration)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dungeon/WaveManager.h"
#include "DungeonRoom.generated.h"

class ABaseEnemy;
class AWaveManager;
class ADoorActor;

UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Treasure UMETA(DisplayName = "Treasure"),
	Boss UMETA(DisplayName = "Boss"),
	Rest UMETA(DisplayName = "Rest")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomCleared, class ADungeonRoom*, Room);

/**
 * 单个地牢房间 — 战斗单元
 *
 * 战斗流程:
 *   1. LevelBuilder 创建房间 + 配置 EnemyClasses (简单) 或 Waves (复杂)
 *   2. DungeonFlow 调用 StartRoom()
 *   3. DungeonRoom 内部创建 WaveManager, 把 EnemyClasses 转成 Waves (如果没自定义)
 *   4. WaveManager 按波次刷怪, 每波之间有间隔
 *   5. 所有波次清完 → OnAllWavesCleared → ClearRoom()
 *   6. DungeonFlow 收到 OnRoomCleared → AdvanceToNextRoom()
 */
UCLASS()
class FIRSTGAME_API ADungeonRoom : public AActor
{
	GENERATED_BODY()

public:
	ADungeonRoom();

	// ─── 基础配置 ────────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	ERoomType RoomType = ERoomType::Combat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	int32 RoomIndex = 0;

	/** 简单模式: 直接罗列所有敌人 Class (会被自动分组为波次) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	TArray<TSubclassOf<ABaseEnemy>> EnemyClasses;

	UPROPERTY(BlueprintReadOnly, Category = "Room")
	bool bIsCleared = false;

	UPROPERTY(BlueprintAssignable, Category = "Room")
	FOnRoomCleared OnRoomCleared;

	/** 出口门 (Clear 后开启, 让玩家进入下一房间) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	ADoorActor* ExitDoor;

	// ─── 波次配置 ────────────────────────────────────────────────────
	/** 高级模式: 自定义波次 (如果设置, 则忽略 EnemyClasses) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	TArray<FWaveConfig> Waves;

	/** 简单模式下, 每波包含的敌人数量 (EnemyClasses 会被按此数量切分成多波) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave", meta=(ClampMin="1"))
	int32 EnemiesPerWave = 3;

	/** 每波敌人之间的 Spawn 间隔 (秒) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	float SpawnDelayBetweenEnemies = 0.5f;

	// ─── 接口 ────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Room")
	void StartRoom();

	UFUNCTION(BlueprintCallable, Category = "Room")
	void ClearRoom();

	/** 当前波次索引 (-1 表示未开始) */
	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetCurrentWaveIndex() const;

	/** 总波次数 */
	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetTotalWaves() const;

protected:
	virtual void BeginPlay() override;

private:
	/** 从 EnemyClasses 自动生成 Waves (简单模式) */
	void BuildWavesFromEnemyClasses();

	/** WaveManager 回调 */
	UFUNCTION()
	void OnWaveStarted(int32 WaveIndex, int32 TotalWaves);

	UFUNCTION()
	void OnWaveCleared(int32 WaveIndex);

	UFUNCTION()
	void OnAllWavesCleared(bool bVictory);

	UPROPERTY()
	AWaveManager* WaveManager;
};
