// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Wave Manager — manages enemy waves within a dungeon room

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Characters/BaseEnemy.h"
#include "WaveManager.generated.h"

class ABaseEnemy;

/** Single wave configuration */
USTRUCT(BlueprintType)
struct FWaveConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	TArray<TSubclassOf<ABaseEnemy>> EnemyClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	int32 EnemyCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	float SpawnDelay = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	bool bIsBossWave = false;

	FWaveConfig() : EnemyCount(3), SpawnDelay(1.0f), bIsBossWave(false) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStarted, int32, WaveIndex, int32, TotalWaves);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCleared, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllWavesCleared, bool, bVictory);

UCLASS()
class FIRSTGAME_API AWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AWaveManager();

	/** Configure waves for this room */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SetupWaves(const TArray<FWaveConfig>& InWaves);

	/** Start the first wave */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWaves();

	/** Get current wave index (0-based, -1 if not started) */
	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	/** Get total wave count */
	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetTotalWaves() const { return Waves.Num(); }

	/** Check if all waves are complete */
	UFUNCTION(BlueprintPure, Category = "Wave")
	bool AreAllWavesComplete() const { return bAllWavesComplete; }

	/** Set default enemy type for this wave manager */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SetDefaultEnemyType(EEnemyType InType) { DefaultEnemyType = InType; }

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveCleared OnWaveCleared;

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnAllWavesCleared OnAllWavesCleared;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnEnemyDied(class ABaseEnemy* Enemy);

	UFUNCTION()
	void SpawnCurrentWave();

	UFUNCTION()
	void CheckWaveClear();

	UPROPERTY()
	TArray<FWaveConfig> Waves;

	UPROPERTY()
	int32 CurrentWaveIndex = -1;

	UPROPERTY()
	bool bAllWavesComplete = false;

	UPROPERTY()
	TArray<ABaseEnemy*> ActiveEnemies;

	/** Default enemy type for spawned enemies */
	UPROPERTY()
	EEnemyType DefaultEnemyType = EEnemyType::CandyZombie;
};
