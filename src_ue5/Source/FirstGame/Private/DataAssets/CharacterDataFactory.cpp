// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "DataAssets/CharacterDataFactory.h"
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
