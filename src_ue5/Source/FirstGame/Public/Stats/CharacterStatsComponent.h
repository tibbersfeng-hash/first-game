// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Character Stats Component — 运行时数值管理 (等级成长/修正/Buff)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/CharacterDataAsset.h"
#include "CharacterStatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, FName, StatName, float, NewValue);

/** 修正类型 */
UENUM(BlueprintType)
enum class EStatModifierType : uint8
{
	Flat        UMETA(DisplayName = "Flat"),         // 固定值加减
	Percent     UMETA(DisplayName = "Percent"),      // 百分比增减 (基于基础值)
	Override    UMETA(DisplayName = "Override")      // 完全覆盖
};

/** 单个数值修正 */
USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SourceTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EStatModifierType Type = EStatModifierType::Flat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Value = 0.f;

	/** 修正持续时间 (0 = 永久) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Duration = 0.f;

	float RemainingTime = 0.f;
	bool bPermanent = true;

	FStatModifier() : SourceTag(NAME_None), Value(0.f), Duration(0.f), RemainingTime(0.f), bPermanent(true) {}

	FStatModifier(FName InTag, EStatModifierType InType, float InValue, float InDuration = 0.f)
		: SourceTag(InTag), Type(InType), Value(InValue), Duration(InDuration),
		  RemainingTime(InDuration), bPermanent(InDuration <= 0.f) {}
};

/** 属性修正列表 (用于 TMap 值类型) */
USTRUCT()
struct FStatModifierList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FStatModifier> Modifiers;
};

/**
 * 角色数值组件 — 管理运行时所有战斗数值
 *
 * 数值计算顺序:
 *   Final = (Base + FlatModifiers) * (1 + PercentModifiers)
 *   若有 Override 修正, 则 Final = OverrideValue
 *
 * 用法:
 *   在 BeginPlay 中 LoadFromDataAsset() 加载基础配置
 *   通过 AddModifier()/RemoveModifier() 添加/移除修正
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class FIRSTGAME_API UCharacterStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStatsComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// ─── 初始化 ──────────────────────────────────────────────────────
	/** 从 DataAsset 加载基础数值 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void LoadFromDataAsset(UCharacterDataAsset* InDataAsset);

	// ─── 等级成长 ────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetLevel(int32 NewLevel);

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetLevel() const { return CharacterLevel; }

	/** 每级成长系数 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Growth")
	float LevelGrowthFactor = 1.05f;

	// ─── 基础数值 (从 DataAsset 加载, 可被等级修正) ──────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Base")
	float BaseMaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats|Base")
	float BaseMaxEnergy = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats|Base")
	float BaseMoveSpeed = 300.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats|Base")
	float BaseJumpForce = 600.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats|Base")
	float BaseKnockbackResistance = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats|Base")
	float BaseEnergyRegenRate = 10.f;

	// ─── 最终数值 (基础 + 等级 + 修正) ───────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxEnergy() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetJumpForce() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetKnockbackResistance() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetEnergyRegenRate() const;

	// ─── 运行时 HP/Energy ────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCurrentEnergy() const { return CurrentEnergy; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetEnergyPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetCurrentHealth(float NewHealth);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetCurrentEnergy(float NewEnergy);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool ConsumeEnergy(float Amount);

	// ─── 修正系统 ────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Stats|Modifiers")
	void AddModifier(FName StatName, const FStatModifier& Modifier);

	UFUNCTION(BlueprintCallable, Category = "Stats|Modifiers")
	void RemoveModifier(FName StatName, FName SourceTag);

	UFUNCTION(BlueprintCallable, Category = "Stats|Modifiers")
	void ClearModifiers(FName StatName);

	UFUNCTION(BlueprintPure, Category = "Stats|Modifiers")
	TArray<FStatModifier> GetModifiers(FName StatName) const;

	// ─── 委托 ────────────────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnStatChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnStatChanged OnEnergyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnStatChanged OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnStatChanged OnMaxEnergyChanged;

private:
	// ─── 内部 ────────────────────────────────────────────────────────
	float CalculateStat(float BaseValue, FName StatName) const;
	float ApplyLevelToBase(float BaseValue) const;
	void InvalidateCache();
	void RemoveExpiredModifiers(float DeltaTime);
	void ClampCurrentValues();

	// ─── 状态 ───────────────────────────────────────────────────────
	UPROPERTY()
	UCharacterDataAsset* DataAsset;

	int32 CharacterLevel = 1;

	float CurrentHealth = 100.f;
	float CurrentEnergy = 100.f;

	/** 原始资产值 (不被等级缩放修改) */
	float AssetMaxHealth = 100.f;
	float AssetMaxEnergy = 100.f;
	float AssetMoveSpeed = 300.f;
	float AssetJumpForce = 600.f;
	float AssetKnockbackResistance = 0.5f;
	float AssetEnergyRegenRate = 10.f;

	/** 缓存的最终数值 (脏位模式) */
	mutable float CachedMaxHealth = -1.f;
	mutable float CachedMaxEnergy = -1.f;
	mutable float CachedMoveSpeed = -1.f;
	mutable float CachedJumpForce = -1.f;
	mutable float CachedKnockbackResistance = -1.f;
	mutable float CachedEnergyRegenRate = -1.f;
	mutable bool bCacheDirty = true;

	/** 各属性名的修正列表 */
	UPROPERTY()
	TMap<FName, FStatModifierList> StatModifiers;
};
