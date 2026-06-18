// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// CharacterStatsComponent Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Stats/CharacterStatsComponent.h"
#include "DataAssets/CharacterDataAsset.h"
#include "DataAssets/CharacterDataFactory.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterStatsComponent Default Configuration
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatsDefaultConfigTest,
	"FirstGame.Stats.CharacterStats.DefaultConfig",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStatsDefaultConfigTest::RunTest(const FString& Parameters)
{
	UCharacterStatsComponent* Stats = NewObject<UCharacterStatsComponent>(GetTransientPackage());
	TestNotNull("StatsComponent should be created", Stats);
	if (!Stats) return false;

	TestEqual("Default level", Stats->GetLevel(), 1);
	TestEqual("Default BaseMaxHealth", Stats->BaseMaxHealth, 100.f);
	TestEqual("Default BaseMaxEnergy", Stats->BaseMaxEnergy, 100.f);
	TestEqual("Default BaseMoveSpeed", Stats->BaseMoveSpeed, 300.f);
	TestEqual("Default LevelGrowthFactor", Stats->LevelGrowthFactor, 1.05f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterStatsComponent LoadFromDataAsset
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatsLoadFromAssetTest,
	"FirstGame.Stats.CharacterStats.LoadFromDataAsset",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStatsLoadFromAssetTest::RunTest(const FString& Parameters)
{
	UCharacterStatsComponent* Stats = NewObject<UCharacterStatsComponent>(GetTransientPackage());
	if (!Stats) return false;

	// Create a test DataAsset
	UCharacterDataAsset* DataAsset = NewObject<UCharacterDataAsset>(GetTransientPackage());
	DataAsset->CharacterId = FName("TestCharacter");
	DataAsset->MaxHealth = 200.f;
	DataAsset->MaxEnergy = 150.f;
	DataAsset->MoveSpeed = 400.f;
	DataAsset->JumpForce = 700.f;
	DataAsset->KnockbackResistance = 0.3f;
	DataAsset->EnergyRegenRate = 15.f;

	Stats->LoadFromDataAsset(DataAsset);

	TestEqual("MaxHealth from asset", Stats->GetMaxHealth(), 200.f);
	TestEqual("MaxEnergy from asset", Stats->GetMaxEnergy(), 150.f);
	TestEqual("MoveSpeed from asset", Stats->GetMoveSpeed(), 400.f);
	TestEqual("JumpForce from asset", Stats->GetJumpForce(), 700.f);
	TestEqual("KnockbackResistance from asset", Stats->GetKnockbackResistance(), 0.3f);
	TestEqual("EnergyRegenRate from asset", Stats->GetEnergyRegenRate(), 15.f);

	// Current values should be initialized to max
	TestEqual("CurrentHealth initialized to max", Stats->GetCurrentHealth(), 200.f);
	TestEqual("CurrentEnergy initialized to max", Stats->GetCurrentEnergy(), 150.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterStatsComponent Heal & Damage
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatsHealDamageTest,
	"FirstGame.Stats.CharacterStats.HealDamage",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStatsHealDamageTest::RunTest(const FString& Parameters)
{
	UCharacterStatsComponent* Stats = NewObject<UCharacterStatsComponent>(GetTransientPackage());
	if (!Stats) return false;

	UCharacterDataAsset* DataAsset = NewObject<UCharacterDataAsset>(GetTransientPackage());
	DataAsset->MaxHealth = 100.f;
	DataAsset->MaxEnergy = 100.f;
	Stats->LoadFromDataAsset(DataAsset);

	// Test damage (SetCurrentHealth below max)
	Stats->SetCurrentHealth(50.f);
	TestEqual("Health after damage", Stats->GetCurrentHealth(), 50.f);
	TestNear("Health percent after damage", Stats->GetHealthPercent(), 0.5f, 0.01f);

	// Test heal
	Stats->Heal(30.f);
	TestEqual("Health after heal", Stats->GetCurrentHealth(), 80.f);

	// Test overheal (should clamp to max)
	Stats->Heal(50.f);
	TestEqual("Health clamped to max", Stats->GetCurrentHealth(), 100.f);

	// Test energy consume
	bool bConsumed = Stats->ConsumeEnergy(30.f);
	TestTrue("Energy consumed successfully", bConsumed);
	TestEqual("Energy after consume", Stats->GetCurrentEnergy(), 70.f);

	// Test insufficient energy
	bConsumed = Stats->ConsumeEnergy(100.f);
	TestFalse("Should not consume more than available", bConsumed);
	TestEqual("Energy unchanged after failed consume", Stats->GetCurrentEnergy(), 70.f);

	// Test death (HP = 0)
	Stats->SetCurrentHealth(0.f);
	TestEqual("Health at death", Stats->GetCurrentHealth(), 0.f);
	TestNear("Health percent at death", Stats->GetHealthPercent(), 0.f, 0.01f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterStatsComponent Flat Modifier
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatsFlatModifierTest,
	"FirstGame.Stats.CharacterStats.FlatModifier",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStatsFlatModifierTest::RunTest(const FString& Parameters)
{
	UCharacterStatsComponent* Stats = NewObject<UCharacterStatsComponent>(GetTransientPackage());
	if (!Stats) return false;

	UCharacterDataAsset* DataAsset = NewObject<UCharacterDataAsset>(GetTransientPackage());
	DataAsset->MaxHealth = 100.f;
	DataAsset->MoveSpeed = 300.f;
	Stats->LoadFromDataAsset(DataAsset);

	// Add flat +50 health modifier
	FStatModifier HealthMod("TestBuff", EStatModifierType::Flat, 50.f);
	Stats->AddModifier("MaxHealth", HealthMod);
	TestEqual("MaxHealth with flat +50", Stats->GetMaxHealth(), 150.f);

	// Add flat -100 speed modifier
	FStatModifier SpeedMod("SlowDebuff", EStatModifierType::Flat, -100.f);
	Stats->AddModifier("MoveSpeed", SpeedMod);
	TestEqual("MoveSpeed with flat -100", Stats->GetMoveSpeed(), 200.f);

	// Remove modifier
	Stats->RemoveModifier("MaxHealth", "TestBuff");
	TestEqual("MaxHealth after removing buff", Stats->GetMaxHealth(), 100.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterStatsComponent Percent Modifier
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatsPercentModifierTest,
	"FirstGame.Stats.CharacterStats.PercentModifier",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStatsPercentModifierTest::RunTest(const FString& Parameters)
{
	UCharacterStatsComponent* Stats = NewObject<UCharacterStatsComponent>(GetTransientPackage());
	if (!Stats) return false;

	UCharacterDataAsset* DataAsset = NewObject<UCharacterDataAsset>(GetTransientPackage());
	DataAsset->MoveSpeed = 300.f;
	Stats->LoadFromDataAsset(DataAsset);

	// Add +50% speed modifier
	FStatModifier SpeedBoost("SpeedBuff", EStatModifierType::Percent, 50.f);
	Stats->AddModifier("MoveSpeed", SpeedBoost);

	// 300 * (1 + 0.5) = 450
	TestEqual("MoveSpeed with +50%", Stats->GetMoveSpeed(), 450.f);

	// Add another +20% modifier
	FStatModifier SpeedBoost2("SpeedBuff2", EStatModifierType::Percent, 20.f);
	Stats->AddModifier("MoveSpeed", SpeedBoost2);

	// 300 * (1 + 0.5 + 0.2) = 510
	TestNear("MoveSpeed with +50% +20%", Stats->GetMoveSpeed(), 510.f, 0.1f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterStatsComponent Override Modifier
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatsOverrideModifierTest,
	"FirstGame.Stats.CharacterStats.OverrideModifier",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStatsOverrideModifierTest::RunTest(const FString& Parameters)
{
	UCharacterStatsComponent* Stats = NewObject<UCharacterStatsComponent>(GetTransientPackage());
	if (!Stats) return false;

	UCharacterDataAsset* DataAsset = NewObject<UCharacterDataAsset>(GetTransientPackage());
	DataAsset->MoveSpeed = 300.f;
	Stats->LoadFromDataAsset(DataAsset);

	// Add override modifier
	FStatModifier OverrideMod("RootEffect", EStatModifierType::Override, 0.f);
	Stats->AddModifier("MoveSpeed", OverrideMod);

	// Override takes priority: speed = 0 regardless of base + other mods
	TestEqual("MoveSpeed with override 0", Stats->GetMoveSpeed(), 0.f);

	// Clear override
	Stats->RemoveModifier("MoveSpeed", "RootEffect");
	TestEqual("MoveSpeed after clearing override", Stats->GetMoveSpeed(), 300.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterStatsComponent Level Scaling
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatsLevelScalingTest,
	"FirstGame.Stats.CharacterStats.LevelScaling",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStatsLevelScalingTest::RunTest(const FString& Parameters)
{
	UCharacterStatsComponent* Stats = NewObject<UCharacterStatsComponent>(GetTransientPackage());
	if (!Stats) return false;

	UCharacterDataAsset* DataAsset = NewObject<UCharacterDataAsset>(GetTransientPackage());
	DataAsset->MaxHealth = 100.f;
	DataAsset->MoveSpeed = 300.f;
	Stats->LevelGrowthFactor = 1.1f; // 10% per level
	Stats->LoadFromDataAsset(DataAsset);

	// Level 1: base values
	TestEqual("Level 1 MaxHealth", Stats->GetMaxHealth(), 100.f);
	TestEqual("Level 1 MoveSpeed", Stats->GetMoveSpeed(), 300.f);

	// Level 2: 100 * 1.1 = 110
	Stats->SetLevel(2);
	TestNear("Level 2 MaxHealth", Stats->GetMaxHealth(), 110.f, 1.f);

	// Level 5: 100 * 1.1^4 = 146.41
	Stats->SetLevel(5);
	TestNear("Level 5 MaxHealth", Stats->GetMaxHealth(), 146.41f, 1.f);

	// Level should be at least 1
	Stats->SetLevel(0);
	TestEqual("Level clamped to minimum 1", Stats->GetLevel(), 1);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterStatsComponent Modifier Priority
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatsModifierPriorityTest,
	"FirstGame.Stats.CharacterStats.ModifierPriority",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStatsModifierPriorityTest::RunTest(const FString& Parameters)
{
	UCharacterStatsComponent* Stats = NewObject<UCharacterStatsComponent>(GetTransientPackage());
	if (!Stats) return false;

	UCharacterDataAsset* DataAsset = NewObject<UCharacterDataAsset>(GetTransientPackage());
	DataAsset->MaxHealth = 100.f;
	Stats->LoadFromDataAsset(DataAsset);

	// Add flat +50
	FStatModifier FlatMod("Flat", EStatModifierType::Flat, 50.f);
	Stats->AddModifier("MaxHealth", FlatMod);

	// Add percent +100%
	FStatModifier PercentMod("Percent", EStatModifierType::Percent, 100.f);
	Stats->AddModifier("MaxHealth", PercentMod);

	// Add override to 200
	FStatModifier OverrideMod("Override", EStatModifierType::Override, 200.f);
	Stats->AddModifier("MaxHealth", OverrideMod);

	// Override wins: should be 200, NOT (100+50)*(1+1.0) = 300
	TestEqual("Override takes priority", Stats->GetMaxHealth(), 200.f);

	// Remove override
	Stats->RemoveModifier("MaxHealth", "Override");
	// Now: (100+50) * (1+1.0) = 300
	TestEqual("Flat+Percent after override removed", Stats->GetMaxHealth(), 300.f);

	return true;
}
