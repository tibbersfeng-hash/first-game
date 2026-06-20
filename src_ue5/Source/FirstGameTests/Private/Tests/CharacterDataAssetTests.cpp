// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// CharacterDataAsset Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "DataAssets/CharacterDataAsset.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterDataAsset Creation & Default Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDataAssetDefaultsTest,
	"FirstGame.DataAssets.CharacterDataAsset.DefaultValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterDataAssetDefaultsTest::RunTest(const FString& Parameters)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(GetTransientPackage());
	TestNotNull("CharacterDataAsset should be created", Data);

	if (!Data) return false;

	// Default stats
	TestEqual("Default MaxHealth", Data->MaxHealth, 100.f);
	TestEqual("Default MaxEnergy", Data->MaxEnergy, 100.f);
	TestEqual("Default MoveSpeed", Data->MoveSpeed, 300.f);
	TestEqual("Default JumpForce", Data->JumpForce, 600.f);
	TestEqual("Default Gravity", Data->Gravity, 2000.f);
	TestEqual("Default MaxJumps", Data->MaxJumps, 2);

	// Default combat values
	TestEqual("Default EnergyRegenRate", Data->EnergyRegenRate, 10.f);
	TestEqual("Default EnergyCostPerAttack", Data->EnergyCostPerAttack, 5.f);
	TestEqual("Default HitStunDuration", Data->HitStunDuration, 0.3f);
	TestEqual("Default KnockbackResistance", Data->KnockbackResistance, 0.5f);

	// Attack arrays should be empty by default
	TestEqual("LightAttacks empty by default", Data->LightAttacks.Num(), 0);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterDataAsset GetPrimaryAssetId
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDataAssetPrimaryIdTest,
	"FirstGame.DataAssets.CharacterDataAsset.PrimaryAssetId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterDataAssetPrimaryIdTest::RunTest(const FString& Parameters)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(GetTransientPackage());
	if (!Data) return false;

	// Set character ID
	Data->CharacterId = FName("Huikong");

	// Get primary asset ID
	FPrimaryAssetId AssetId = Data->GetPrimaryAssetId();

	// Should have correct type and name
	TestEqual("Asset type should be Character", AssetId.PrimaryAssetType.GetName(), FName("Character"));
	TestEqual("Asset name should be Huikong", AssetId.PrimaryAssetName, FName("Huikong"));

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CharacterDataAsset Custom Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDataAssetCustomValuesTest,
	"FirstGame.DataAssets.CharacterDataAsset.CustomValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterDataAssetCustomValuesTest::RunTest(const FString& Parameters)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(GetTransientPackage());
	if (!Data) return false;

	// Configure as Huikong (balanced character from GDD)
	Data->CharacterId = FName("Huikong");
	Data->DisplayName = FText::FromString("悟空");
	Data->MaxHealth = 100.f;
	Data->MaxEnergy = 100.f;
	Data->MoveSpeed = 320.f;
	Data->MaxJumps = 2;

	TestEqual("Huikong MaxHealth", Data->MaxHealth, 100.f);
	TestEqual("Huikong MoveSpeed", Data->MoveSpeed, 320.f);
	TestEqual("Huikong MaxJumps", Data->MaxJumps, 2);

	// Configure as Tangtang (high energy, 3 jumps)
	UCharacterDataAsset* Tangtang = NewObject<UCharacterDataAsset>(GetTransientPackage());
	Tangtang->CharacterId = FName("Tangtang");
	Tangtang->MaxHealth = 80.f;
	Tangtang->MaxEnergy = 120.f;
	Tangtang->MaxJumps = 3;
	Tangtang->EnergyRegenRate = 15.f;

	TestEqual("Tangtang MaxHealth", Tangtang->MaxHealth, 80.f);
	TestEqual("Tangtang MaxEnergy", Tangtang->MaxEnergy, 120.f);
	TestEqual("Tangtang MaxJumps", Tangtang->MaxJumps, 3);
	TestEqual("Tangtang EnergyRegenRate", Tangtang->EnergyRegenRate, 15.f);

	// Configure as Kiguemaru (tank, high damage, low speed)
	UCharacterDataAsset* Kiguemaru = NewObject<UCharacterDataAsset>(GetTransientPackage());
	Kiguemaru->CharacterId = FName("Kiguemaru");
	Kiguemaru->MaxHealth = 130.f;
	Kiguemaru->MoveSpeed = 260.f;
	Kiguemaru->MaxJumps = 1;

	TestEqual("Kiguemaru MaxHealth", Kiguemaru->MaxHealth, 130.f);
	TestEqual("Kiguemaru MoveSpeed", Kiguemaru->MoveSpeed, 260.f);
	TestEqual("Kiguemaru MaxJumps", Kiguemaru->MaxJumps, 1);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: FAttackMoveData Default Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackMoveDataDefaultsTest,
	"FirstGame.DataAssets.AttackMoveData.DefaultValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAttackMoveDataDefaultsTest::RunTest(const FString& Parameters)
{
	FAttackMoveData AttackData;

	// Default values from struct definition
	TestEqual("Default Damage", AttackData.Damage, 10.f);
	TestEqual("Default StartupFrames", AttackData.StartupFrames, 3.f);
	TestEqual("Default ActiveFrames", AttackData.ActiveFrames, 4.f);
	TestEqual("Default RecoveryFrames", AttackData.RecoveryFrames, 6.f);
	TestEqual("Default HitStopFrames", AttackData.HitStopFrames, 5.f);
	TestFalse("Default bLaunchesEnemy", AttackData.bLaunchesEnemy);
	TestEqual("Default LaunchForce", AttackData.LaunchForce, 200.f);
	TestEqual("Default Knockback X", (double)AttackData.Knockback.X, (double)100.);
	TestEqual("Default Knockback Y", (double)AttackData.Knockback.Y, (double)-50.);
	TestEqual("Default MoveName", AttackData.MoveName, FName(NAME_None));

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: FAttackMoveData Custom Configuration
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackMoveDataCustomTest,
	"FirstGame.DataAssets.AttackMoveData.CustomConfiguration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAttackMoveDataCustomTest::RunTest(const FString& Parameters)
{
	FAttackMoveData LightAttack;
	LightAttack.MoveName = "LightAttack1";
	LightAttack.Damage = 8.f;
	LightAttack.StartupFrames = 2.f;
	LightAttack.ActiveFrames = 3.f;
	LightAttack.RecoveryFrames = 5.f;
	LightAttack.HitStopFrames = 4.f;
	LightAttack.Knockback = FVector2D(80.f, -30.f);

	TestEqual("Custom MoveName", LightAttack.MoveName, FName("LightAttack1"));
	TestEqual("Custom Damage", LightAttack.Damage, 8.f);
	TestEqual("Custom StartupFrames", LightAttack.StartupFrames, 2.f);
	TestEqual("Custom Knockback X", (double)LightAttack.Knockback.X, (double)80.);

	// Heavy attack configuration
	FAttackMoveData HeavyAttack;
	HeavyAttack.MoveName = "HeavyAttack";
	HeavyAttack.Damage = 25.f;
	HeavyAttack.StartupFrames = 8.f;
	HeavyAttack.ActiveFrames = 6.f;
	HeavyAttack.RecoveryFrames = 12.f;
	HeavyAttack.HitStopFrames = 8.f;
	HeavyAttack.bLaunchesEnemy = true;
	HeavyAttack.LaunchForce = 400.f;

	TestEqual("Heavy Damage", HeavyAttack.Damage, 25.f);
	TestTrue("Heavy launches enemy", HeavyAttack.bLaunchesEnemy);
	TestEqual("Heavy LaunchForce", HeavyAttack.LaunchForce, 400.f);

	return true;
}
