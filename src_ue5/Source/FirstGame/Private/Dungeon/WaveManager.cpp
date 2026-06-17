// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Dungeon/WaveManager.h"
#include "Characters/BaseEnemy.h"
#include "Subsystems/SignalBusFunctionLibrary.h"

AWaveManager::AWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWaveManager::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("WaveManager initialized (%d waves configured)"), Waves.Num());
}

void AWaveManager::SetupWaves(const TArray<FWaveConfig>& InWaves)
{
	Waves = InWaves;
	CurrentWaveIndex = -1;
	bAllWavesComplete = false;
	UE_LOG(LogTemp, Log, TEXT("WaveManager: %d waves configured"), Waves.Num());
}

void AWaveManager::StartWaves()
{
	if (Waves.Num() == 0)
	{
		bAllWavesComplete = true;
		OnAllWavesCleared.Broadcast(true);
		return;
	}

	CurrentWaveIndex = 0;
	SpawnCurrentWave();
}

void AWaveManager::SpawnCurrentWave()
{
	if (CurrentWaveIndex < 0 || CurrentWaveIndex >= Waves.Num()) return;

	const FWaveConfig& Wave = Waves[CurrentWaveIndex];
	UE_LOG(LogTemp, Log, TEXT("Wave %d/%d starting (%d enemies, boss=%s)"),
		CurrentWaveIndex + 1, Waves.Num(), Wave.EnemyCount, Wave.bIsBossWave ? TEXT("YES") : TEXT("NO"));

	OnWaveStarted.Broadcast(CurrentWaveIndex, Waves.Num());

	// Spawn enemies with delay
	for (int32 i = 0; i < Wave.EnemyCount; i++)
	{
		FTimerHandle SpawnHandle;
		float Delay = Wave.SpawnDelay * i;

		GetWorldTimerManager().SetTimer(SpawnHandle, [this, Wave, i]()
		{
			// Pick enemy class (cycle through available classes)
			if (Wave.EnemyClasses.Num() == 0) return;

			TSubclassOf<ABaseEnemy> EnemyClass = Wave.EnemyClasses[i % Wave.EnemyClasses.Num()];
			FVector SpawnPos = GetActorLocation() + FVector(FMath::RandRange(-200.f, 200.f), 0.f, 0.f);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			ABaseEnemy* Enemy = GetWorld()->SpawnActor<ABaseEnemy>(EnemyClass, SpawnPos, FRotator::ZeroRotator, SpawnParams);
			if (Enemy)
			{
				ActiveEnemies.Add(Enemy);
				Enemy->OnEnemyDied.AddDynamic(this, &AWaveManager::OnEnemyDied);
			}
		}, Delay, false);
	}
}

void AWaveManager::OnEnemyDied(ABaseEnemy* Enemy)
{
	ActiveEnemies.Remove(Enemy);
	CheckWaveClear();
}

void AWaveManager::CheckWaveClear()
{
	// Check if all enemies in current wave are dead
	bool bWaveCleared = true;
	for (ABaseEnemy* Enemy : ActiveEnemies)
	{
		if (Enemy && Enemy->GetCurrentState() != "Dead")
		{
			bWaveCleared = false;
			break;
		}
	}

	if (bWaveCleared && ActiveEnemies.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Wave %d cleared!"), CurrentWaveIndex + 1);
		OnWaveCleared.Broadcast(CurrentWaveIndex);

		ActiveEnemies.Empty();

		// Move to next wave
		CurrentWaveIndex++;
		if (CurrentWaveIndex >= Waves.Num())
		{
			bAllWavesComplete = true;
			UE_LOG(LogTemp, Log, TEXT("All waves complete!"));
			OnAllWavesCleared.Broadcast(true);

			USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
			if (SignalBus)
			{
				SignalBus->OnDungeonCompleted.Broadcast(true);
			}
		}
		else
		{
			// Brief pause between waves, then spawn next
			FTimerHandle Handle;
			GetWorldTimerManager().SetTimer(Handle, [this]()
			{
				SpawnCurrentWave();
			}, 2.0f, false);
		}
	}
}
