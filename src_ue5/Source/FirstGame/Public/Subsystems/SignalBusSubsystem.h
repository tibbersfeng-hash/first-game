// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// ADR-003: Signal Bus — 全局事件广播系统

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/SignalBusTypes.h"
#include "SignalBusSubsystem.generated.h"

/**
 * Global event bus for decoupled communication between systems.
 * Replaces Godot's Signal pattern with UE5 Dynamic Multicast Delegates.
 */
UCLASS()
class FIRSTGAME_API USignalBusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ─── Combat Events ───────────────────────────────────────────────

	/** Player landed a hit on an enemy */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHitLanded, AActor*, Attacker, AActor*, Target, float, Damage, FVector, HitLocation);
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHitLanded OnHitLanded;

	/** A combo was completed */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboFinished, AActor*, Player, int32, ComboCount);
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnComboFinished OnComboFinished;

	/** Combo counter updated */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboUpdated, AActor*, Player, int32, CurrentCount);
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnComboUpdated OnComboUpdated;

	/** Hit stop requested (freeze frames) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitStopRequested, float, Duration);
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHitStopRequested OnHitStopRequested;

	// ─── Player Events ───────────────────────────────────────────────

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerHealthChanged, AActor*, Player, float, NewHealth);
	UPROPERTY(BlueprintAssignable, Category = "Player")
	FOnPlayerHealthChanged OnPlayerHealthChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnergyChanged, AActor*, Player, float, NewEnergy);
	UPROPERTY(BlueprintAssignable, Category = "Player")
	FOnPlayerEnergyChanged OnPlayerEnergyChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDied, AActor*, Player);
	UPROPERTY(BlueprintAssignable, Category = "Player")
	FOnPlayerDied OnPlayerDied;

	// ── Enemy Events ────────────────────────────────────────────────

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDied, AActor*, Enemy);
	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyDied OnEnemyDied;

	// ─── Dungeon Events ──────────────────────────────────────────────

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomCleared, AActor*, Room);
	UPROPERTY(BlueprintAssignable, Category = "Dungeon")
	FOnRoomCleared OnRoomCleared;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomEntered, AActor*, Room);
	UPROPERTY(BlueprintAssignable, Category = "Dungeon")
	FOnRoomEntered OnRoomEntered;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonCompleted, bool, bVictory);
	UPROPERTY(BlueprintAssignable, Category = "Dungeon")
	FOnDungeonCompleted OnDungeonCompleted;

	// ─── UI Events ───────────────────────────────────────────────────

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShowDamageNumber, const FDamageNumberData&, Data);
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnShowDamageNumber OnShowDamageNumber;
};
