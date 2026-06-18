// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Character Stats Component — 运行时数值管理实现

#include "Stats/CharacterStatsComponent.h"

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
		if (RegenRate > 0.f && CurrentEnergy < GetMaxEnergy())
		{
			float OldEnergy = CurrentEnergy;
			CurrentEnergy = FMath::Clamp(CurrentEnergy + RegenRate * DeltaTime, 0.f, GetMaxEnergy());
			if (!FMath::IsNearlyEqual(OldEnergy, CurrentEnergy))
			{
				OnEnergyChanged.Broadcast("EnergyRegen", CurrentEnergy);
			}
		}
	}
}

// ─── 初始化 ─────────────────────────────────────────────────────────

void UCharacterStatsComponent::LoadFromDataAsset(UCharacterDataAsset* InDataAsset)
{
	DataAsset = InDataAsset;
	if (!DataAsset) { return; }

	// 加载基础数值
	BaseMaxHealth = DataAsset->MaxHealth;
	BaseMaxEnergy = DataAsset->MaxEnergy;
	BaseMoveSpeed = DataAsset->MoveSpeed;
	BaseJumpForce = DataAsset->JumpForce;
	BaseKnockbackResistance = DataAsset->KnockbackResistance;
	BaseEnergyRegenRate = DataAsset->EnergyRegenRate;

	// 应用等级成长
	ApplyLevelScaling();

	// 初始化运行时值
	CurrentHealth = GetMaxHealth();
	CurrentEnergy = GetMaxEnergy();

	UE_LOG(LogTemp, Log, TEXT("Stats: Loaded from %s (HP:%.0f, Energy:%.0f, Speed:%.0f)"),
		*DataAsset->CharacterId.ToString(), GetMaxHealth(), GetMaxEnergy(), GetMoveSpeed());
}

// ─── 等级成长 ───────────────────────────────────────────────────────

void UCharacterStatsComponent::SetLevel(int32 NewLevel)
{
	CharacterLevel = FMath::Max(1, NewLevel);
	ApplyLevelScaling();

	// 等级提升时补满 HP/Energy
	CurrentHealth = GetMaxHealth();
	CurrentEnergy = GetMaxEnergy();

	OnMaxHealthChanged.Broadcast("LevelUp", GetMaxHealth());
	OnMaxEnergyChanged.Broadcast("LevelUp", GetMaxEnergy());

	UE_LOG(LogTemp, Log, TEXT("Stats: Level %d → %d (MaxHP:%.0f)"), CharacterLevel - 1, CharacterLevel, GetMaxHealth());
}

// ─── 最终数值计算 ──────────────────────────────────────────────────

float UCharacterStatsComponent::GetMaxHealth() const
{
	return CalculateStat(BaseMaxHealth, "MaxHealth");
}

float UCharacterStatsComponent::GetMaxEnergy() const
{
	return CalculateStat(BaseMaxEnergy, "MaxEnergy");
}

float UCharacterStatsComponent::GetMoveSpeed() const
{
	return CalculateStat(BaseMoveSpeed, "MoveSpeed");
}

float UCharacterStatsComponent::GetJumpForce() const
{
	return CalculateStat(BaseJumpForce, "JumpForce");
}

float UCharacterStatsComponent::GetKnockbackResistance() const
{
	return CalculateStat(BaseKnockbackResistance, "KnockbackResistance");
}

float UCharacterStatsComponent::GetEnergyRegenRate() const
{
	return CalculateStat(BaseEnergyRegenRate, "EnergyRegenRate");
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
	if (CurrentEnergy < Amount) { return false; }
	SetCurrentEnergy(CurrentEnergy - Amount);
	return true;
}

// ─── 修正系统 ─────────────────────────────────────────────────────

void UCharacterStatsComponent::AddModifier(FName StatName, const FStatModifier& Modifier)
{
	FStatModifierList& List = StatModifiers.FindOrAdd(StatName);

	// 移除同来源的旧修正
	List.Modifiers.RemoveAll([&Modifier](const FStatModifier& M) { return M.SourceTag == Modifier.SourceTag; });

	List.Modifiers.Add(Modifier);

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
		UE_LOG(LogTemp, Verbose, TEXT("Stats: Removed %d modifier(s) '%s' from %s"),
			Removed, *SourceTag.ToString(), *StatName.ToString());
	}
}

void UCharacterStatsComponent::ClearModifiers(FName StatName)
{
	StatModifiers.Remove(StatName);
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

	// Override 优先级最高
	if (bHasOverride) { return OverrideValue; }

	// Final = (Base + Flat) * (1 + Percent)
	return (BaseValue + FlatSum) * (1.f + PercentSum);
}

void UCharacterStatsComponent::ApplyLevelScaling()
{
	if (CharacterLevel <= 1) { return; }

	// 等级成长公式: Base * GrowthFactor^(Level-1)
	float GrowthMultiplier = FMath::Pow(LevelGrowthFactor, CharacterLevel - 1);

	BaseMaxHealth *= GrowthMultiplier;
	BaseMaxEnergy *= GrowthMultiplier;
	BaseMoveSpeed *= (1.f + (GrowthMultiplier - 1.f) * 0.3f); // 移速成长较缓
	BaseJumpForce *= GrowthMultiplier;
}

void UCharacterStatsComponent::RemoveExpiredModifiers(float DeltaTime)
{
	for (auto It = StatModifiers.CreateIterator(); It; ++It)
	{
		FStatModifierList& List = It.Value();
		List.Modifiers.RemoveAll([DeltaTime](FStatModifier& M)
		{
			if (M.bPermanent) { return false; }
			M.RemainingTime -= DeltaTime;
			return M.RemainingTime <= 0.f;
		});

		if (List.Modifiers.Num() == 0)
		{
			It.RemoveCurrent();
		}
	}

	// 修正后钳制当前值
	ClampCurrentValues();
}

void UCharacterStatsComponent::ClampCurrentValues()
{
	float MaxHP = GetMaxHealth();
	float MaxEnergy = GetMaxEnergy();

	if (CurrentHealth > MaxHP) { CurrentHealth = MaxHP; }
	if (CurrentEnergy > MaxEnergy) { CurrentEnergy = MaxEnergy; }
}
