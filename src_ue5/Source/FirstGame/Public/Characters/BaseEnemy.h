// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Base Enemy — AI-controlled combatant

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

class UAbilitySystemComponent;
class UHitBoxComponent;
class UHurtBoxComponent;
class UCharacterDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyStateChanged, class ABaseEnemy*, Enemy, FName, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDied, class ABaseEnemy*, Enemy);

UCLASS()
class FIRSTGAME_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseEnemy();

	UFUNCTION(BlueprintCallable, Category = "Setup")
	void InitializeEnemy(UCharacterDataAsset* InDataAsset);

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	UCharacterDataAsset* EnemyData;

	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnEnemyStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnEnemyDied OnEnemyDied;

	UFUNCTION(BlueprintCallable, Category = "State")
	FName GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ReceiveHitDamage(float Amount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void Die();

	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsAggro() const { return bIsAggro; }

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetAggro(bool bInAggro) { bIsAggro = bInAggro; }

	// ─── 掉落配置 ────────────────────────────────────────────────────
	/** 死亡时掉落物品的概率 (0-1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	float DropChance = 0.3f;

	/** 掉落物品的基础值 (HP 恢复量 / 能量恢复量) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	float LootValue = 20.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UHitBoxComponent* EnemyHitBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UHurtBoxComponent* EnemyHurtBox;

private:
	void SetState(FName NewState);

	UPROPERTY()
	FName CurrentState = "Idle";

	UPROPERTY()
	bool bIsAggro = false;

	UPROPERTY()
	float AggroRange = 500.f;

	UPROPERTY()
	float AttackRange = 80.f;

	UPROPERTY()
	float AttackCooldown = 1.5f;

	UPROPERTY()
	float LastAttackTime = 0.f;
};
