// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Level Builder — 程序化生成可玩关卡
// 把空关卡变成可玩关卡的唯一 Actor：地板、墙、灯光、地牢流程、玩家出生点、HUD 一气呵成

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelBuilder.generated.h"

class ADungeonFlow;
class ADungeonRoom;
class ABaseEnemy;
class APlayerStart;
class APlayerCharacter;
class UCameraComponent;
class UUserWidget;
class UCharacterDataAsset;

/**
 * 程序化生成可玩关卡。
 *
 * 用法:
 *   1. 在空 Level 中放置 1 个 LevelBuilder Actor（或让 GameMode 自动 Spawn）
 *   2. 在 Details 面板配置：房间数量、敌人种类、房间尺寸、HUD Class
 *   3. PIE 即可进入一个完整可玩的关卡
 *
 * 职责（单一 Actor，所有东西都是它的子 Actor）:
 *   - 场地几何（地板 + 围墙 + 装饰柱）
 *   - 灯光（1 个方向光 + N 个点光）
 *   - 玩家出生点
 *   - DungeonFlow + N 个 DungeonRoom
 *   - HUD Widget（挂到 PlayerController）
 *
 * 清理:
 *   - 所有 Spawn 出的子 Actor 都加 "LevelBuilder.Child" Tag
 *   - Destroy 时会一并清理
 */
UCLASS(Blueprintable, meta = (DisplayName = "Level Builder"))
class FIRSTGAME_API ALevelBuilder : public AActor
{
	GENERATED_BODY()

public:
	ALevelBuilder();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ─── 配置：场地 ─────────────────────────────────────────────────
	/** 单个房间的尺寸（X=宽, Y=长, Z=高）单位 cm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
	FVector RoomExtent = FVector(3000.f, 4000.f, 600.f);

	/** 房间之间的通道长度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
	float RoomGapDistance = 1500.f;

	/** 地板厚度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
	float FloorThickness = 50.f;

	/** 围墙厚度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
	float WallThickness = 100.f;

	/** 是否生成装饰柱 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
	bool bSpawnPillars = true;

	// ─── 配置：地牢流程 ──────────────────────────────────────────────
	/** 总房间数（至少 1）*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon", meta = (ClampMin = "1"))
	int32 TotalRooms = 3;

	/** 每个房间的基础敌人数 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon", meta = (ClampMin = "0"))
	int32 BaseEnemyCountPerRoom = 3;

	/** 默认敌人 Class（若 DungeonRoom 没配就用这个）*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon")
	TSubclassOf<ABaseEnemy> DefaultEnemyClass;

	/** 最终房间是否是 Boss 房（多 2 倍敌人）*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon")
	bool bBossOnFinalRoom = true;

	// ─── 配置：灯光 ─────────────────────────────────────────────────
	/** 方向光强度（lux）*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lighting")
	float DirectionalLightIntensity = 5000.f;

	/** 方向光颜色 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lighting")
	FLinearColor DirectionalLightColor = FLinearColor(1.f, 0.95f, 0.9f, 1.f);

	/** 方向光旋转（Yaw/Pitch/Roll）*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lighting")
	FRotator DirectionalLightRotation = FRotator(-45.f, 30.f, 0.f);

	// ─── 配置：HUD ───────────────────────────────────────────────────
	/** HUD Widget Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	// ─── 配置：玩家角色 ──────────────────────────────────────────────
	/** 默认角色 ID (在 CharacterDataFactory 里注册的角色) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	FName DefaultCharacterId = FName(TEXT("Huikong"));

	// ─── 配置：调试 ──────────────────────────────────────────────────
	/** 调试模式：打印所有 Spawn 信息 + 显示房间边界 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	bool bDebugMode = false;

	// ─── 运行时状态 ─────────────────────────────────────────────────
	/** 生成的 DungeonFlow（便于外部访问）*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	ADungeonFlow* DungeonFlow;

	/** 生成的所有房间 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	TArray<ADungeonRoom*> SpawnedRooms;

	/** 所有 Spawn 出的子 Actor（用于清理）*/
	UPROPERTY()
	TArray<AActor*> ChildActors;

protected:
	// ─── 子步骤 ──────────────────────────────────────────────────────
	virtual void BuildArenaGeometry();
	virtual void BuildLighting();
	virtual void SetupPlayerStart();
	virtual void BuildDungeonFlow();
	virtual void SetupHUD();
	virtual void InitializePlayer();

	/** 生成单个房间的几何（地板 + 四面墙 + 可选柱子）*/
	void BuildRoomGeometry(int32 RoomIndex, const FVector& RoomOrigin);

	/** 生成单个 DungeonRoom Actor */
	ADungeonRoom* BuildDungeonRoom(int32 RoomIndex, const FVector& RoomOrigin);

	/** 工具：Spawn 一个立方体几何 Actor */
	AActor* SpawnCube(const FVector& Center, const FVector& Extent,
	                  const FLinearColor& Color, FName TagSuffix);

	/** 工具：清理所有子 Actor */
	void CleanupChildren();

	/** 记录调试日志 */
	void LogBuild(const FString& Message) const;
};
