// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "DataAssets/CharacterDataFactory.h"
#include "FirstGame.h"
#include "DataAssets/CharacterDataAsset.h"

// ─── GDD Default Values ──────────────────────────────────────────────
// Source: design/gdd/combat-system.md

FAttackMoveData UCharacterDataFactory::GetDefaultLightAttack(int32 Index)
{
	FAttackMoveData Move;
	switch (Index)
	{
	case 0: // Light 1
		Move.MoveName = "Light1";
		Move.Damage = 10.f;
		Move.StartupFrames = 2.f;
		Move.ActiveFrames = 3.f;
		Move.RecoveryFrames = 4.f;
		Move.HitStopFrames = 4.f;
		Move.Knockback = FVector2D(80.f, -30.f);
		break;
	case 1: // Light 2
		Move.MoveName = "Light2";
		Move.Damage = 12.f;
		Move.StartupFrames = 2.f;
		Move.ActiveFrames = 3.f;
		Move.RecoveryFrames = 4.f;
		Move.HitStopFrames = 5.f;
		Move.Knockback = FVector2D(100.f, -40.f);
		break;
	case 2: // Light 3 (launcher)
		Move.MoveName = "Light3";
		Move.Damage = 15.f;
		Move.StartupFrames = 3.f;
		Move.ActiveFrames = 4.f;
		Move.RecoveryFrames = 5.f;
		Move.HitStopFrames = 6.f;
		Move.Knockback = FVector2D(60.f, -200.f);
		Move.bLaunchesEnemy = true;
		Move.LaunchForce = 400.f;
		break;
	}
	return Move;
}

FAttackMoveData UCharacterDataFactory::GetDefaultHeavyAttack()
{
	FAttackMoveData Move;
	Move.MoveName = "Heavy";
	Move.Damage = 25.f;
	Move.StartupFrames = 5.f;
	Move.ActiveFrames = 4.f;
	Move.RecoveryFrames = 8.f;
	Move.HitStopFrames = 8.f;
	Move.Knockback = FVector2D(200.f, -80.f);
	Move.bLaunchesEnemy = true;
	Move.LaunchForce = 300.f;
	return Move;
}

FAttackMoveData UCharacterDataFactory::GetDefaultSpecialMove()
{
	FAttackMoveData Move;
	Move.MoveName = "Special";
	Move.Damage = 50.f;
	Move.StartupFrames = 8.f;
	Move.ActiveFrames = 6.f;
	Move.RecoveryFrames = 12.f;
	Move.HitStopFrames = 12.f;
	Move.Knockback = FVector2D(300.f, -150.f);
	Move.bLaunchesEnemy = true;
	Move.LaunchForce = 600.f;
	return Move;
}

// ─── Character-Specific Data ─────────────────────────────────────────

UCharacterDataAsset* UCharacterDataFactory::CreateHuikongData(UObject* Outer)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(Outer);
	Data->CharacterId = "Huikong";
	Data->DisplayName = FText::FromString(TEXT("慧空 · 气功小武僧"));

	// Stats — 武僧: 均衡型
	Data->MaxHealth = 100.f;
	Data->MaxEnergy = 100.f;
	Data->MoveSpeed = 320.f;
	Data->JumpForce = 650.f;
	Data->Gravity = 2000.f;
	Data->MaxJumps = 2;
	Data->EnergyRegenRate = 8.f;
	Data->EnergyCostPerAttack = 5.f;

	// Attacks
	Data->LightAttacks.Add(GetDefaultLightAttack(0));
	Data->LightAttacks.Add(GetDefaultLightAttack(1));
	Data->LightAttacks.Add(GetDefaultLightAttack(2));
	Data->HeavyAttack = GetDefaultHeavyAttack();
	Data->SpecialMove = GetDefaultSpecialMove();

	// Dodge
	Data->DodgeMove.MoveName = "Dodge";
	Data->DodgeMove.StartupFrames = 2.f;
	Data->DodgeMove.ActiveFrames = 8.f;  // i-frames
	Data->DodgeMove.RecoveryFrames = 4.f;

	// Defense
	Data->HitStunDuration = 0.3f;
	Data->KnockbackResistance = 0.5f;

	UE_LOG(LogTemp, Log, TEXT("Created Huikong DataAsset (HP:%.0f, SPD:%.0f)"), Data->MaxHealth, Data->MoveSpeed);
	return Data;
}

UCharacterDataAsset* UCharacterDataFactory::CreateTangtangData(UObject* Outer)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(Outer);
	Data->CharacterId = "Tangtang";
	Data->DisplayName = FText::FromString(TEXT("糖糖 · 糖果魔导师"));

	// Stats — 法师: 脆皮高伤，高能量
	Data->MaxHealth = 80.f;
	Data->MaxEnergy = 120.f;
	Data->MoveSpeed = 280.f;
	Data->JumpForce = 580.f;
	Data->Gravity = 1800.f;
	Data->MaxJumps = 3;  // 法师可以三段跳（糖果浮空）
	Data->EnergyRegenRate = 12.f;
	Data->EnergyCostPerAttack = 4.f;

	// Attacks — 魔法攻击距离稍远
	Data->LightAttacks.Add(GetDefaultLightAttack(0));
	Data->LightAttacks[0].Damage = 8.f;    // 糖果弹伤害较低
	Data->LightAttacks.Add(GetDefaultLightAttack(1));
	Data->LightAttacks[1].Damage = 10.f;
	Data->LightAttacks.Add(GetDefaultLightAttack(2));
	Data->LightAttacks[2].Damage = 12.f;
	Data->HeavyAttack = GetDefaultHeavyAttack();
	Data->HeavyAttack.Damage = 20.f;       // 地雷伤害
	Data->HeavyAttack.StartupFrames = 8.f; // 放置时间更长
	Data->SpecialMove = GetDefaultSpecialMove();
	Data->SpecialMove.Damage = 45.f;       // 糖果风暴

	// Dodge — 魔法闪现
	Data->DodgeMove.MoveName = "TeleportDodge";
	Data->DodgeMove.StartupFrames = 1.f;
	Data->DodgeMove.ActiveFrames = 10.f;
	Data->DodgeMove.RecoveryFrames = 3.f;

	Data->HitStunDuration = 0.25f;
	Data->KnockbackResistance = 0.3f;

	UE_LOG(LogTemp, Log, TEXT("Created Tangtang DataAsset (HP:%.0f, Energy:%.0f)"), Data->MaxHealth, Data->MaxEnergy);
	return Data;
}

UCharacterDataAsset* UCharacterDataFactory::CreateKiguemaruData(UObject* Outer)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(Outer);
	Data->CharacterId = "Kiguemaru";
	Data->DisplayName = FText::FromString(TEXT("小鬼丸 · 萌鬼狂战士"));

	// Stats — 狂战士: 高血高伤，低速
	Data->MaxHealth = 130.f;
	Data->MaxEnergy = 80.f;
	Data->MoveSpeed = 260.f;
	Data->JumpForce = 550.f;
	Data->Gravity = 2200.f;
	Data->MaxJumps = 1;  // 狂战士只能跳一次（重）
	Data->EnergyRegenRate = 6.f;
	Data->EnergyCostPerAttack = 6.f;

	// Attacks — 重击型
	Data->LightAttacks.Add(GetDefaultLightAttack(0));
	Data->LightAttacks[0].Damage = 14.f;   // 金棒挥击伤害高
	Data->LightAttacks[0].StartupFrames = 3.f;
	Data->LightAttacks.Add(GetDefaultLightAttack(1));
	Data->LightAttacks[1].Damage = 16.f;
	Data->LightAttacks.Add(GetDefaultLightAttack(2));
	Data->LightAttacks[2].Damage = 20.f;
	Data->LightAttacks[2].StartupFrames = 4.f;
	Data->HeavyAttack = GetDefaultHeavyAttack();
	Data->HeavyAttack.Damage = 35.f;       // 跳砸超高伤害
	Data->HeavyAttack.StartupFrames = 7.f;
	Data->HeavyAttack.Knockback = FVector2D(250.f, -120.f);
	Data->SpecialMove = GetDefaultSpecialMove();
	Data->SpecialMove.Damage = 60.f;       // 鬼王降临最高伤害
	Data->SpecialMove.StartupFrames = 10.f;

	// Dodge — 冲刺闪避
	Data->DodgeMove.MoveName = "DashDodge";
	Data->DodgeMove.StartupFrames = 3.f;
	Data->DodgeMove.ActiveFrames = 6.f;
	Data->DodgeMove.RecoveryFrames = 5.f;

	Data->HitStunDuration = 0.35f;
	Data->KnockbackResistance = 0.7f;

	UE_LOG(LogTemp, Log, TEXT("Created Kiguemaru DataAsset (HP:%.0f, DMG:%.0f)"), Data->MaxHealth, Data->MaxHealth);
	return Data;
}

UCharacterDataAsset* UCharacterDataFactory::CreateDefaultEnemyData(UObject* Outer)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(Outer);
	if (!Data) return nullptr;

	// 基础敌人数据 (CandyZombie 风格)
	// GDD: design/gdd/enemy-ai.md
	Data->CharacterId = FName(TEXT("CandyZombie"));
	Data->DisplayName = FText::FromString(TEXT("糖果僵尸"));

	// Stats (CandyZombie: 慢速弱鸡)
	Data->MaxHealth = 30.f;
	Data->MaxEnergy = 0.f;
	Data->MoveSpeed = 150.f;
	Data->JumpForce = 0.f;
	Data->Gravity = 2000.f;
	Data->EnergyRegenRate = 0.f;
	Data->EnergyCostPerAttack = 0.f;

	// Light attack (简单的一击)
	FAttackMoveData EnemyAttack;
	EnemyAttack.MoveName = FName(TEXT("Bite"));
	EnemyAttack.Damage = 8.f;
	EnemyAttack.StartupFrames = 5.f;      // 出招慢
	EnemyAttack.ActiveFrames = 4.f;
	EnemyAttack.RecoveryFrames = 8.f;     // 收招慢
	EnemyAttack.HitStopFrames = 3.f;
	EnemyAttack.Knockback = FVector2D(150.f, 50.f);
	EnemyAttack.bLaunchesEnemy = false;
	Data->LightAttacks.Add(EnemyAttack);

	// Heavy attack (无)
	Data->HeavyAttack = FAttackMoveData();
	Data->HeavyAttack.MoveName = FName(TEXT("None"));

	// Special move (无)
	Data->SpecialMove = FAttackMoveData();
	Data->SpecialMove.MoveName = FName(TEXT("None"));

	// Dodge (无)
	Data->DodgeMove = FAttackMoveData();
	Data->DodgeMove.MoveName = FName(TEXT("None"));

	Data->HitStunDuration = 0.3f;
	Data->KnockbackResistance = 0.2f;    // 轻甲, 容易被击退

	UE_LOG(LogTemp, Log, TEXT("Created Default Enemy DataAsset (HP:%.0f, DMG:%.0f)"),
		Data->MaxHealth, EnemyAttack.Damage);
	return Data;
}

UCharacterDataAsset* UCharacterDataFactory::CreateGingerbreadData(UObject* Outer)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(Outer);
	if (!Data) return nullptr;

	// 姜饼人 — 中等敌人，速度适中，血量适中
	Data->CharacterId = FName(TEXT("Gingerbread"));
	Data->DisplayName = FText::FromString(TEXT("姜饼人"));

	Data->MaxHealth = 50.f;
	Data->MaxEnergy = 0.f;
	Data->MoveSpeed = 220.f;
	Data->JumpForce = 400.f;
	Data->Gravity = 2000.f;
	Data->EnergyRegenRate = 0.f;
	Data->EnergyCostPerAttack = 0.f;

	// Light attack — 拳击
	FAttackMoveData Attack;
	Attack.MoveName = FName(TEXT("Punch"));
	Attack.Damage = 10.f;
	Attack.StartupFrames = 4.f;
	Attack.ActiveFrames = 4.f;
	Attack.RecoveryFrames = 6.f;
	Attack.HitStopFrames = 3.f;
	Attack.Knockback = FVector2D(180.f, 60.f);
	Attack.bLaunchesEnemy = false;
	Data->LightAttacks.Add(Attack);

	Data->HeavyAttack = FAttackMoveData();
	Data->HeavyAttack.MoveName = FName(TEXT("None"));
	Data->SpecialMove = FAttackMoveData();
	Data->SpecialMove.MoveName = FName(TEXT("None"));
	Data->DodgeMove = FAttackMoveData();
	Data->DodgeMove.MoveName = FName(TEXT("None"));

	Data->HitStunDuration = 0.35f;
	Data->KnockbackResistance = 0.3f;

	UE_LOG(LogTemp, Log, TEXT("Created Gingerbread DataAsset (HP:%.0f, SPD:%.0f)"),
		Data->MaxHealth, Data->MoveSpeed);
	return Data;
}

UCharacterDataAsset* UCharacterDataFactory::CreateShadowNinjaData(UObject* Outer)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(Outer);
	if (!Data) return nullptr;

	// 暗影忍者 — 快速低血量，高闪避
	Data->CharacterId = FName(TEXT("ShadowNinja"));
	Data->DisplayName = FText::FromString(TEXT("暗影忍者"));

	Data->MaxHealth = 35.f;
	Data->MaxEnergy = 0.f;
	Data->MoveSpeed = 350.f;
	Data->JumpForce = 600.f;
	Data->Gravity = 2000.f;
	Data->EnergyRegenRate = 0.f;
	Data->EnergyCostPerAttack = 0.f;

	// Light attack — 快速匕首
	FAttackMoveData Attack;
	Attack.MoveName = FName(TEXT("Dagger"));
	Attack.Damage = 8.f;
	Attack.StartupFrames = 2.f;      // 出招快
	Attack.ActiveFrames = 3.f;
	Attack.RecoveryFrames = 4.f;     // 收招快
	Attack.HitStopFrames = 2.f;
	Attack.Knockback = FVector2D(120.f, 40.f);
	Attack.bLaunchesEnemy = false;
	Data->LightAttacks.Add(Attack);

	Data->HeavyAttack = FAttackMoveData();
	Data->HeavyAttack.MoveName = FName(TEXT("None"));
	Data->SpecialMove = FAttackMoveData();
	Data->SpecialMove.MoveName = FName(TEXT("None"));
	Data->DodgeMove = FAttackMoveData();
	Data->DodgeMove.MoveName = FName(TEXT("None"));

	Data->HitStunDuration = 0.25f;   // 硬直短
	Data->KnockbackResistance = 0.1f; // 轻甲，容易被击退

	UE_LOG(LogTemp, Log, TEXT("Created ShadowNinja DataAsset (HP:%.0f, SPD:%.0f)"),
		Data->MaxHealth, Data->MoveSpeed);
	return Data;
}

UCharacterDataAsset* UCharacterDataFactory::CreateArmoredGumData(UObject* Outer)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(Outer);
	if (!Data) return nullptr;

	// 装甲口香糖 — 慢速高血量高防御
	Data->CharacterId = FName(TEXT("ArmoredGum"));
	Data->DisplayName = FText::FromString(TEXT("装甲口香糖"));

	Data->MaxHealth = 100.f;
	Data->MaxEnergy = 0.f;
	Data->MoveSpeed = 120.f;
	Data->JumpForce = 300.f;
	Data->Gravity = 2400.f;          // 重，跳得低
	Data->EnergyRegenRate = 0.f;
	Data->EnergyCostPerAttack = 0.f;

	// Light attack — 重锤
	FAttackMoveData Attack;
	Attack.MoveName = FName(TEXT("Hammer"));
	Attack.Damage = 15.f;
	Attack.StartupFrames = 6.f;      // 出招慢
	Attack.ActiveFrames = 5.f;
	Attack.RecoveryFrames = 10.f;    // 收招慢
	Attack.HitStopFrames = 4.f;
	Attack.Knockback = FVector2D(250.f, 80.f);
	Attack.bLaunchesEnemy = true;    // 可以击飞
	Attack.LaunchForce = 300.f;
	Data->LightAttacks.Add(Attack);

	Data->HeavyAttack = FAttackMoveData();
	Data->HeavyAttack.MoveName = FName(TEXT("None"));
	Data->SpecialMove = FAttackMoveData();
	Data->SpecialMove.MoveName = FName(TEXT("None"));
	Data->DodgeMove = FAttackMoveData();
	Data->DodgeMove.MoveName = FName(TEXT("None"));

	Data->HitStunDuration = 0.4f;
	Data->KnockbackResistance = 0.7f; // 重甲，难击退

	UE_LOG(LogTemp, Log, TEXT("Created ArmoredGum DataAsset (HP:%.0f, DMG:%.0f, DEF:%.1f)"),
		Data->MaxHealth, Attack.Damage, Data->KnockbackResistance);
	return Data;
}
