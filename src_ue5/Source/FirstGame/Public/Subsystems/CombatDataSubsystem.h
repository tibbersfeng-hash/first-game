// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// ADR-004: Resource-Based Data Configuration — 战斗数据配置

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataAssets/CharacterDataAsset.h"
#include "CombatDataSubsystem.generated.h"

/**
 * Central repository for combat-related data.
 * Stores character stats, move data, combo tables.
 * Replaces Godot's CombatData autoload singleton.
 */
UCLASS()
class FIRSTGAME_API UCombatDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ─── Character Data Access ───────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Combat Data")
	UCharacterDataAsset* GetCharacterData(FName CharacterId) const;

	UFUNCTION(BlueprintCallable, Category = "Combat Data")
	void RegisterCharacterData(FName CharacterId, UCharacterDataAsset* Data);

	// ─── Combat Constants ────────────────────────────────────────────

	/** Default hit stop duration in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float DefaultHitStopDuration = 0.08f;

	/** Default hit freeze duration in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float DefaultHitFreezeDuration = 0.15f;

	/** Maximum combo count before reset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 MaxComboCount = 99;

	/** Combo reset timeout in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float ComboResetTimeout = 1.5f;

	/** Frame rate target for frame-based calculations */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 TargetFrameRate = 60;

private:
	/** Character data lookup table */
	UPROPERTY()
	TMap<FName, UCharacterDataAsset*> CharacterDataMap;
};
