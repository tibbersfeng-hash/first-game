// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGameGameMode.h"
#include "FirstGame.h"
#include "Characters/PlayerCharacter.h"
#include "DataAssets/CharacterDataFactory.h"
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
	// 等待玩家生成后初始化
	FTimerHandle InitHandle;
	GetWorldTimerManager().SetTimer(InitHandle, [this]()
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (!Player)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: PlayerCharacter not found yet"));
			return;
		}

		if (Player->CharacterData)
		{
			UE_LOG(LogTemp, Log, TEXT("GameMode: Player already initialized"));
			return;
		}

		// 创建武僧数据
		UCharacterDataAsset* HuikongData = UCharacterDataFactory::CreateHuikongData(this);
		if (HuikongData)
		{
			Player->InitializeCharacter(HuikongData);
			UE_LOG(LogTemp, Log, TEXT("GameMode: Player initialized with Huikong data"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameMode: Failed to create Huikong data"));
		}
	}, 0.5f, false);  // 等待 0.5 秒让玩家生成
}
