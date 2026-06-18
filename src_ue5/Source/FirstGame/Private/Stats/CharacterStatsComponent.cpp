// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Character Stats Component — 运行时数值管理实现

#include "Stats/CharacterStatsComponent.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "Subsystems/SignalBusSubsystem.h"

UCharacterStatsComponent::UCharacterStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // 每 0.5s 清理过期修正
}

void UCharacterStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RemoveExpiredModifiers(DeltaTime);

	// 能量自动回复
	if (DataAsset)
	{
		float RegenRate = GetEnergyRegenRate();
		float MaxEnergy = GetMaxEnergy();
		if (RegenRate > 0.f && CurrentEnergy < MaxEnergy)
		{
			float OldEnergy = CurrentEnergy;
			CurrentEnergy = FMath::Clamp(CurrentEnergy + RegenRate * DeltaTime, 0.f, MaxEnergy);
			if (!FMath::IsNearlyEqual(OldEnergy, CurrentEnergy))
			{
				OnEnergyChanged.Broadcast("EnergyRegen", CurrentEnergy);

				// Bug #4 fix: 同时广播到 SignalBus，确保 HUD 能收到挂机回能事件
				USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
				if (SignalBus)
				{
					SignalBus->OnPlayerEnergyChanged.Broadcast(GetOwner(), CurrentEnergy);
				}
			}
		}
	}
}

// ─── 初始化 ─────────────────────────────────────────────────────────

void UCharacterStatsComponent::LoadFromDataAsset(UCharacterDataAsset* InDataAsset)
{
	DataAsset = InDataAsset;
	if (!DataAsset) { return; }

	// 保存原始资产值 (Bug #1 fix: 不被等级缩放原地修改)
	AssetMaxHealth = DataAsset->MaxHealth;
	AssetMaxEnergy = DataAsset->MaxEnergy;
	AssetMoveSpeed = DataAsset->MoveSpeed;
	AssetJumpForce = DataAsset->JumpForce;
	AssetKnockbackResistance = DataAsset->KnockbackResistance;
	AssetEnergyRegenRate = DataAsset->EnergyRegenRate;

	// 同步 Base* 字段 (供蓝图/编辑器查看)
	BaseMaxHealth = ApplyLevelToBase(AssetMaxHealth);
	BaseMaxEnergy = ApplyLevelToBase(AssetMaxEnergy);
	BaseMoveSpeed = ApplyLevelToBase(AssetMoveSpeed);
	BaseJumpForce = ApplyLevelToBase(AssetJumpForce);
	BaseKnockbackResistance = ApplyLevelToBase(AssetKnockbackResistance);
	BaseEnergyRegenRate = ApplyLevelToBase(AssetEnergyRegenRate);

	InvalidateCache();

	// 初始化运行时值
	CurrentHealth = GetMaxHealth();
	CurrentEnergy = GetMaxEnergy();

	UE_LOG(LogTemp, Log, TEXT("Stats: Loaded from %s (HP:%.0f, Energy:%.0f, Speed:%.0f)"),
		*DataAsset->CharacterId.ToString(), GetMaxHealth(), GetMaxEnergy(), GetMoveSpeed());
}

// ─── 等级成长 ───────────────────────────────────────────────────────

void UCharacterStatsComponent::SetLevel(int32 NewLevel)
{
	int32 OldLevel = CharacterLevel;
	CharacterLevel = FMath::Max(1, NewLevel);

	// Bug #1 fix: 从原始资产值重新计算, 不原地累积
	BaseMaxHealth = ApplyLevelToBase(AssetMaxHealth);
	BaseMaxEnergy = ApplyLevelToBase(AssetMaxEnergy);
	BaseMoveSpeed = ApplyLevelToBase(AssetMoveSpeed);
	BaseJumpForce = ApplyLevelToBase(AssetJumpForce);
	BaseKnockbackResistance = ApplyLevelToBase(AssetKnockbackResistance);
	BaseEnergyRegenRate = ApplyLevelToBase(AssetEnergyRegenRate);

	InvalidateCache();

	// 等级提升时补满 HP/Energy
	CurrentHealth = GetMaxHealth();
	CurrentEnergy = GetMaxEnergy();

	OnMaxHealthChanged.Broadcast("LevelUp", GetMaxHealth());
	OnMaxEnergyChanged.Broadcast("LevelUp", GetMaxEnergy());

	UE_LOG(LogTemp, Log, TEXT("Stats: Level %d -> %d (MaxHP:%.0f)"), OldLevel, CharacterLevel, GetMaxHealth());
}

// ─── 最终数值计算 (Bug #7 fix: 脏位缓存) ───────────────────────────

float UCharacterStatsComponent::GetMaxHealth() const
{
	if (bCacheDirty) { CachedMaxHealth = CalculateStat(BaseMaxHealth, "MaxHealth"); }
	return CachedMaxHealth;
}

float UCharacterStatsComponent::GetMaxEnergy() const
{
	if (bCacheDirty) { CachedMaxEnergy = CalculateStat(BaseMaxEnergy, "MaxEnergy"); }
	return CachedMaxEnergy;
}

float UCharacterStatsComponent::GetMoveSpeed() const
{
	if (bCacheDirty) { CachedMoveSpeed = CalculateStat(BaseMoveSpeed, "MoveSpeed"); }
	return CachedMoveSpeed;
}

float UCharacterStatsComponent::GetJumpForce() const
{
	if (bCacheDirty) { CachedJumpForce = CalculateStat(BaseJumpForce, "JumpForce"); }
	return CachedJumpForce;
}

float UCharacterStatsComponent::GetKnockbackResistance() const
{
	if (bCacheDirty) { CachedKnockbackResistance = CalculateStat(BaseKnockbackResistance, "KnockbackResistance"); }
	return CachedKnockbackResistance;
}

float UCharacterStatsComponent::GetEnergyRegenRate() const
{
	if (bCacheDirty) { CachedEnergyRegenRate = CalculateStat(BaseEnergyRegenRate, "EnergyRegenRate"); }
	return CachedEnergyRegenRate;
}

// ─── 运行时 HP/Energy ─────────────────────────────────────────────

float UCharacterStatsComponent::GetHealthPercent() const
{
	float MaxHP = GetMaxHealth();
	return MaxHP > 0.f ? FMath::Clamp(CurrentHealth / MaxHP, 0.f, 1.f) : 0.f;
}

float UCharacterStatsComponent::GetEnergyPercent() const
{
	float MaxEnergy = GetMaxEnergy();
	return MaxEnergy > 0.f ? FMath::Clamp(CurrentEnergy / MaxEnergy, 0.f, 1.f) : 0.f;
}

void UCharacterStatsComponent::SetCurrentHealth(float NewHealth)
{
	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(NewHealth, 0.f, GetMaxHealth());
	if (!FMath::IsNearlyEqual(OldHealth, CurrentHealth))
	{
		OnHealthChanged.Broadcast("SetHealth", CurrentHealth);
	}
}

void UCharacterStatsComponent::SetCurrentEnergy(float NewEnergy)
{
	float OldEnergy = CurrentEnergy;
	CurrentEnergy = FMath::Clamp(NewEnergy, 0.f, GetMaxEnergy());
	if (!FMath::IsNearlyEqual(OldEnergy, CurrentEnergy))
	{
		OnEnergyChanged.Broadcast("SetEnergy", CurrentEnergy);
	}
}

void UCharacterStatsComponent::Heal(float Amount)
{
	if (Amount <= 0.f) { return; }
	SetCurrentHealth(CurrentHealth + Amount);
}

bool UCharacterStatsComponent::ConsumeEnergy(float Amount)
{
	if (CurrentEnergy < Amount)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Stats: Not enough energy: need %.0f, have %.0f"), Amount, CurrentEnergy);
		return false;
	}
	SetCurrentEnergy(CurrentEnergy - Amount);
	return true;
}

// ── 修正系统 ─────────────────────────────────────────────────────

void UCharacterStatsComponent::AddModifier(FName StatName, const FStatModifier& Modifier)
{
	FStatModifierList& List = StatModifiers.FindOrAdd(StatName);
	List.Modifiers.RemoveAll([&Modifier](const FStatModifier& M) { return M.SourceTag == Modifier.SourceTag; });
	List.Modifiers.Add(Modifier);

	InvalidateCache(); // Bug #7 fix: 修正变化时使缓存失效

	UE_LOG(LogTemp, Verbose, TEXT("Stats: +%s modifier '%s' (%.1f)"),
		*StatName.ToString(), *Modifier.SourceTag.ToString(), Modifier.Value);
}

void UCharacterStatsComponent::RemoveModifier(FName StatName, FName SourceTag)
{
	FStatModifierList* List = StatModifiers.Find(StatName);
	if (!List) { return; }

	int32 Removed = List->Modifiers.RemoveAll([SourceTag](const FStatModifier& M) { return M.SourceTag == SourceTag; });
	if (Removed > 0)
	{
		InvalidateCache(); // Bug #7 fix
		UE_LOG(LogTemp, Verbose, TEXT("Stats: Removed %d modifier(s) '%s' from %s"),
			Removed, *SourceTag.ToString(), *StatName.ToString());
	}
}

void UCharacterStatsComponent::ClearModifiers(FName StatName)
{
	if (StatModifiers.Contains(StatName))
	{
		StatModifiers.Remove(StatName);
		InvalidateCache();
	}
}

TArray<FStatModifier> UCharacterStatsComponent::GetModifiers(FName StatName) const
{
	const FStatModifierList* List = StatModifiers.Find(StatName);
	return List ? List->Modifiers : TArray<FStatModifier>();
}

// ─── 内部 ─────────────────────────────────────────────────────────

float UCharacterStatsComponent::CalculateStat(float BaseValue, FName StatName) const
{
	const FStatModifierList* List = StatModifiers.Find(StatName);
	if (!List || List->Modifiers.Num() == 0) { return BaseValue; }

	float FlatSum = 0.f;
	float PercentSum = 0.f;
	bool bHasOverride = false;
	float OverrideValue = 0.f;

	for (const FStatModifier& Mod : List->Modifiers)
	{
		if (Mod.bPermanent || Mod.RemainingTime > 0.f)
		{
			switch (Mod.Type)
			{
			case EStatModifierType::Flat:
				FlatSum += Mod.Value;
				break;
			case EStatModifierType::Percent:
				PercentSum += Mod.Value / 100.f;
				break;
			case EStatModifierType::Override:
				bHasOverride = true;
				OverrideValue = Mod.Value;
				break;
			}
		}
	}

	if (bHasOverride) { return OverrideValue; }
	return (BaseValue + FlatSum) * (1.f + PercentSum);
}

// Bug #1 fix: 从原始值计算等级缩放, 不修改 Base* 字段
float UCharacterStatsComponent::ApplyLevelToBase(float BaseValue) const
{
	if (CharacterLevel <= 1) { return BaseValue; }
	// Bug #8 helper: 移速成长较缓
	return BaseValue * FMath::Pow(LevelGrowthFactor, CharacterLevel - 1);
}

void UCharacterStatsComponent::InvalidateCache()
{
	bCacheDirty = true;
	CachedMaxHealth = -1.f;
	CachedMaxEnergy = -1.f;
	CachedMoveSpeed = -1.f;
	CachedJumpForce = -1.f;
	CachedKnockbackResistance = -1.f;
	CachedEnergyRegenRate = -1.f;
}

void UCharacterStatsComponent::RemoveExpiredModifiers(float DeltaTime)
{
	bool bAnyExpired = false;
	for (auto It = StatModifiers.CreateIterator(); It; ++It)
	{
		FStatModifierList& List = It.Value();
		List.Modifiers.RemoveAll([DeltaTime, &bAnyExpired](FStatModifier& M)
		{
			if (M.bPermanent) { return false; }
			M.RemainingTime -= DeltaTime;
			bool bExpired = M.RemainingTime <= 0.f;
			if (bExpired) { bAnyExpired = true; }
			return bExpired;
		});

		if (List.Modifiers.Num() == 0)
		{
			It.RemoveCurrent();
		}
	}

	// Bug #7 fix: 仅在修正实际过期时才重算和钳制
	if (bAnyExpired)
	{
		InvalidateCache();
		ClampCurrentValues();
	}
}

void UCharacterStatsComponent::ClampCurrentValues()
{
	float MaxHP = GetMaxHealth();
	float MaxEnergy = GetMaxEnergy();

	if (CurrentHealth > MaxHP) { CurrentHealth = MaxHP; }
	if (CurrentEnergy > MaxEnergy) { CurrentEnergy = MaxEnergy; }
}
