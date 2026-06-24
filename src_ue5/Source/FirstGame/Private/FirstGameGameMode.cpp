// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGameGameMode.h"
#include "FirstGame.h"
#include "Characters/PlayerCharacter.h"
#include "DataAssets/CharacterDataFactory.h"
#include "DataAssets/CharacterDataAsset.h"
#include "Level/LevelBuilder.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AFirstGameGameMode::AFirstGameGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();

	// 默认启用 LevelBuilder — 让空关卡直接变成可玩关卡
	// 用户可在蓝图子类中覆盖 LevelBuilderClass 指向自定义配置
	LevelBuilderClass = ALevelBuilder::StaticClass();
}

void AFirstGameGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("FirstGame GameMode started — 格斗萌主"));

	// 创建并初始化玩家数据
	InitializePlayerCharacter();

	// 如果指定了 LevelBuilderClass，则自动 Spawn 一个构建可玩关卡
	if (LevelBuilderClass)
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("GameMode: GetWorld() 失败, 无法 Spawn LevelBuilder"));
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ActiveLevelBuilder = World->SpawnActor<ALevelBuilder>(
			LevelBuilderClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (ActiveLevelBuilder)
		{
			UE_LOG(LogTemp, Log, TEXT("GameMode: 已 Spawn LevelBuilder — 关卡构建中..."));
			UE_LOG(LogTemp, Log,
				TEXT("GameMode: 若要自定义关卡配置, 创建 LevelBuilder 蓝图子类并设置 Class Defaults"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameMode: LevelBuilder Spawn 失败"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("GameMode: 未配置 LevelBuilderClass, 需要手动在关卡中放置 LevelBuilder Actor"));
	}
}

void AFirstGameGameMode::InitializePlayerCharacter()
{
	// 使用重试机制等待玩家生成
	PlayerInitRetryCount = 0;
	RetryInitializePlayer();
}

void AFirstGameGameMode::RetryInitializePlayer()
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player)
	{
		PlayerInitRetryCount++;
		if (PlayerInitRetryCount < MaxPlayerInitRetries)
		{
			UE_LOG(LogTemp, Log, TEXT("GameMode: 等待玩家生成 (%d/%d)"), PlayerInitRetryCount, MaxPlayerInitRetries);
			FTimerHandle Handle;
			GetWorldTimerManager().SetTimer(Handle, this, &AFirstGameGameMode::RetryInitializePlayer, 0.3f, false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameMode: 玩家生成超时，无法初始化数据"));
		}
		return;
	}

	if (Player->CharacterData)
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: 玩家已初始化"));
		return;
	}

	// 创建武僧数据
	UCharacterDataAsset* HuikongData = UCharacterDataFactory::CreateHuikongData(this);
	if (HuikongData)
	{
		Player->InitializeCharacter(HuikongData);
		UE_LOG(LogTemp, Log, TEXT("GameMode: 玩家初始化成功 (HP:%.0f, Speed:%.0f)"),
			HuikongData->MaxHealth, HuikongData->MoveSpeed);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: 创建 Huikong 数据失败"));
	}
}
