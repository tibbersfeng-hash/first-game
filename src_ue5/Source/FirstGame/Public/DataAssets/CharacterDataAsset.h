// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// ADR-004: Resource-Based Data Configuration — 角色数据资产

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterDataAsset.generated.h"

/** Attack move data */
USTRUCT(BlueprintType)
struct FAttackMoveData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FName MoveName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float StartupFrames = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float ActiveFrames = 4.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float RecoveryFrames = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float HitStopFrames = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FVector2D Knockback = FVector2D(100.f, -50.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	bool bLaunchesEnemy = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float LaunchForce = 200.f;

	FAttackMoveData() : MoveName(NAME_None) {}
};

/** Character stats and configuration data asset */
UCLASS(BlueprintType, Blueprintable)
class FIRSTGAME_API UCharacterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ── Basic Info ──────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FName CharacterId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	TSoftObjectPtr<UTexture2D> PortraitTexture;

	// ── Stats ────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxEnergy = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MoveSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float JumpForce = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float Gravity = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	int32 MaxJumps = 2;

	// ─── Combat ──────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<FAttackMoveData> LightAttacks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FAttackMoveData HeavyAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FAttackMoveData SpecialMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FAttackMoveData DodgeMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float EnergyRegenRate = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float EnergyCostPerAttack = 5.f;

	// ─── Defense ─────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float HitStunDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float KnockbackResistance = 0.5f;

	// ─── UPrimaryDataAsset Interface ─────────────────────────────────

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("Character", CharacterId);
	}
};
