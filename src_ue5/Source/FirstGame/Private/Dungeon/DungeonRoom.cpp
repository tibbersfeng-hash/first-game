// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Dungeon/DungeonRoom.h"
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
}

void ADungeonRoom::StartRoom()
{
	if (bIsCleared) return;

	UE_LOG(LogTemp, Log, TEXT("Room %d started! Spawning %d enemies"), RoomIndex, EnemyClasses.Num());

	for (TSubclassOf<ABaseEnemy> EnemyClass : EnemyClasses)
	{
		if (!EnemyClass) continue;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ABaseEnemy* Enemy = GetWorld()->SpawnActor<ABaseEnemy>(EnemyClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Enemy)
		{
			SpawnedEnemies.Add(Enemy);
			Enemy->OnEnemyDied.AddDynamic(this, &ADungeonRoom::OnEnemyDied);
		}
	}
}

void ADungeonRoom::CheckRoomClear()
{
	if (bIsCleared) return;

	bool bAllDead = true;
	for (ABaseEnemy* Enemy : SpawnedEnemies)
	{
		if (Enemy && Enemy->GetCurrentState() != "Dead")
		{
			bAllDead = false;
			break;
		}
	}

	if (bAllDead && SpawnedEnemies.Num() > 0)
	{
		ClearRoom();
	}
}

void ADungeonRoom::ClearRoom()
{
	bIsCleared = true;
	UE_LOG(LogTemp, Log, TEXT("Room %d CLEARED!"), RoomIndex);

	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnRoomCleared.Broadcast(this);
	}

	OnRoomCleared.Broadcast(this);
}

void ADungeonRoom::OnEnemyDied(ABaseEnemy* Enemy)
{
	CheckRoomClear();
}
