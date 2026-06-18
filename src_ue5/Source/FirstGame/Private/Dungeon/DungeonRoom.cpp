// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Dungeon/DungeonRoom.h"
#include "Dungeon/WaveManager.h"
#include "Characters/BaseEnemy.h"
#include "Subsystems/SignalBusFunctionLibrary.h"

ADungeonRoom::ADungeonRoom()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADungeonRoom::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("DungeonRoom %d initialized (Type: %d)"), RoomIndex, (int)RoomType);

	// 创建 WaveManager (挂在房间位置, 敌人在此附近生成)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	WaveManager = GetWorld()->SpawnActor<AWaveManager>(
		AWaveManager::StaticClass(),
		GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!WaveManager)
	{
		UE_LOG(LogTemp, Error, TEXT("DungeonRoom %d: WaveManager Spawn 失败"), RoomIndex);
	}
}

void ADungeonRoom::StartRoom()
{
	if (bIsCleared)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonRoom %d: 已经 Clear, 忽略 StartRoom"), RoomIndex);
		return;
	}

	if (!WaveManager)
	{
		UE_LOG(LogTemp, Error, TEXT("DungeonRoom %d: WaveManager 为 null, 无法启动"), RoomIndex);
		return;
	}

	// 如果没自定义 Waves, 从 EnemyClasses 自动生成
	if (Waves.Num() == 0 && EnemyClasses.Num() > 0)
	{
		BuildWavesFromEnemyClasses();
	}

	if (Waves.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("DungeonRoom %d: 没配置 Waves 也没配置 EnemyClasses, 直接 Clear"), RoomIndex);
		ClearRoom();
		return;
	}

	UE_LOG(LogTemp, Log,
		TEXT("DungeonRoom %d: StartRoom — 共 %d 波敌人"), RoomIndex, Waves.Num());

	// 绑定 WaveManager 的委托
	WaveManager->OnWaveStarted.AddDynamic(this, &ADungeonRoom::OnWaveStarted);
	WaveManager->OnWaveCleared.AddDynamic(this, &ADungeonRoom::OnWaveCleared);
	WaveManager->OnAllWavesCleared.AddDynamic(this, &ADungeonRoom::OnAllWavesCleared);

	// 配置并启动波次
	WaveManager->SetupWaves(Waves);
	WaveManager->StartWaves();
}

void ADungeonRoom::ClearRoom()
{
	if (bIsCleared) return;

	bIsCleared = true;
	UE_LOG(LogTemp, Log, TEXT("DungeonRoom %d CLEARED!"), RoomIndex);

	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnRoomCleared.Broadcast(this);
	}

	OnRoomCleared.Broadcast(this);
}

int32 ADungeonRoom::GetCurrentWaveIndex() const
{
	return WaveManager ? WaveManager->GetCurrentWaveIndex() : -1;
}

int32 ADungeonRoom::GetTotalWaves() const
{
	return WaveManager ? WaveManager->GetTotalWaves() : Waves.Num();
}

void ADungeonRoom::BuildWavesFromEnemyClasses()
{
	// 把 EnemyClasses 按 EnemiesPerWave 切分成多波
	// 例: EnemyClasses=[G,G,G,O,O,O], EnemiesPerWave=3
	//   → Wave 1: EnemyClasses=[G] x 3, Wave 2: EnemyClasses=[O] x 3
	//
	// 例: EnemyClasses=[G,O,B], EnemiesPerWave=2
	//   → Wave 1: EnemyClasses=[G,O] x 2 (交替), Wave 2: EnemyClasses=[B] x 1

	FWaveConfig CurrentWave;
	CurrentWave.SpawnDelay = SpawnDelayBetweenEnemies;
	CurrentWave.EnemyCount = 0;

	// Boss 房的最后一波特殊处理
	const bool bIsBossRoom = (RoomType == ERoomType::Boss);

	for (int32 i = 0; i < EnemyClasses.Num(); i++)
	{
		TSubclassOf<ABaseEnemy> EnemyClass = EnemyClasses[i];
		if (!EnemyClass) continue;

		// 如果这个 Class 还没在当前波里, 加进去 (去重)
		if (!CurrentWave.EnemyClasses.Contains(EnemyClass))
		{
			CurrentWave.EnemyClasses.Add(EnemyClass);
		}
		CurrentWave.EnemyCount++;

		// 达到 EnemiesPerWave, 或者最后一个敌人, 提交这一波
		const bool bIsLastEnemy = (i == EnemyClasses.Num() - 1);
		if (CurrentWave.EnemyCount >= EnemiesPerWave || bIsLastEnemy)
		{
			// Boss 房的最后一波标记为 BossWave
			if (bIsBossRoom && bIsLastEnemy)
			{
				CurrentWave.bIsBossWave = true;
			}
			Waves.Add(CurrentWave);

			// 重置, 准备下一波
			CurrentWave = FWaveConfig();
			CurrentWave.SpawnDelay = SpawnDelayBetweenEnemies;
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("DungeonRoom %d: 自动生成 %d 波 (敌人总数 %d, 每波 %d)"),
		RoomIndex, Waves.Num(), EnemyClasses.Num(), EnemiesPerWave);
}

void ADungeonRoom::OnWaveStarted(int32 WaveIndex, int32 TotalWaves)
{
	UE_LOG(LogTemp, Log,
		TEXT("DungeonRoom %d: Wave %d/%d 开始!"),
		RoomIndex, WaveIndex + 1, TotalWaves);
}

void ADungeonRoom::OnWaveCleared(int32 WaveIndex)
{
	UE_LOG(LogTemp, Log,
		TEXT("DungeonRoom %d: Wave %d 清除"),
		RoomIndex, WaveIndex + 1);
}

void ADungeonRoom::OnAllWavesCleared(bool bVictory)
{
	UE_LOG(LogTemp, Log,
		TEXT("DungeonRoom %d: 所有波次完成 (%s), 清理房间"),
		RoomIndex, bVictory ? TEXT("胜利") : TEXT("失败"));
	ClearRoom();
}
