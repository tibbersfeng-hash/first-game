// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Base Enemy — AI-controlled combatant

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LockOn/ILockableTarget.h"
#include "BaseEnemy.generated.h"

class UAbilitySystemComponent;
class UHitBoxComponent;
class UHurtBoxComponent;
class UCharacterDataAsset;
class UMonsterAnimInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyStateChanged, class ABaseEnemy*, Enemy, FName, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDied, class ABaseEnemy*, Enemy);

/** 敌人类型 */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	CandyZombie    UMETA(DisplayName = "Candy Zombie"),
	Gingerbread    UMETA(DisplayName = "Gingerbread"),
	ShadowNinja    UMETA(DisplayName = "Shadow Ninja"),
	ArmoredGum     UMETA(DisplayName = "Armored Gum")
};

UCLASS()
class FIRSTGAME_API ABaseEnemy : public ACharacter, public ILockableTarget
{
	GENERATED_BODY()

public:
	ABaseEnemy();

	/** 敌人类型 (决定使用哪种数据) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	EEnemyType EnemyType = EEnemyType::CandyZombie;

	/** 获取 EnemyType 数值 (给 MonsterAnimInstance 用) */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	uint8 GetEnemyTypeValue() const { return static_cast<uint8>(EnemyType); }

	UFUNCTION(BlueprintCallable, Category = "Setup")
	void InitializeEnemy(UCharacterDataAsset* InDataAsset);

	/** 根据 EnemyType 自动加载对应的 SkeletalMesh + AnimBP */
	UFUNCTION(BlueprintCallable, Category = "Setup")
	void ConfigureMonsterAssets();

	/** 配置 LOD 设置 */
	void ConfigureLODSettings();

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

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void Die();

	// ─── 动画控制 ───────────────────────────────────────────────────

	/** 获取 MonsterAnimInstance */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	UMonsterAnimInstance* GetMonsterAnimInstance() const;

	/** 触发受击动画 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayHitAnimation();

	/** 触发死亡动画 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayDeathAnimation();

	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsAggro() const { return bIsAggro; }

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetAggro(bool bInAggro) { bIsAggro = bInAggro; }

	// ─── ILockableTarget 实现 ────────────────────────────────────────
	virtual float GetHPRatio_Implementation() const override;
	virtual bool IsLockable_Implementation() const override;
	virtual float GetLockPriority_Implementation() const override;

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
