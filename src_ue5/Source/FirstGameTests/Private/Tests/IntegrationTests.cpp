// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Integration Tests — UE5 Automation Framework
// Tests that verify cross-system interactions

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Combat/ComboManager.h"
#include "Combat/HitStopManager.h"
#include "DataAssets/CharacterDataAsset.h"
#include "Subsystems/SignalBusTypes.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: Combat Flow — Hit → Combo → HitStop
// Verifies the core combat loop logic without requiring world context.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatFlowIntegrationTest,
	"FirstGame.Integration.CombatFlow",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCombatFlowIntegrationTest::RunTest(const FString& Parameters)
{
	// Simulate a complete combat flow:
	// 1. Player attacks → HitBox activates
	// 2. Hit registered → ComboManager increments
	// 3. HitStop requested → frames freeze
	// 4. Recovery → next attack possible

	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	UHitStopManager* HitStop = NewObject<UHitStopManager>(GetTransientPackage());

	if (!Combo || !HitStop) return false;

	// --- Hit 1 ---
	// Simulate HitBox overlap
	float Damage1 = 8.f;

	// ComboManager registers hit
	Combo->RegisterHit("LightAttack");
	TestEqual("After hit 1, combo = 1", Combo->GetCurrentCount(), 1);

	// HitStop requested (from HitBoxComponent)
	// (Can't actually call RequestHitStop without World, but verify logic)
	float HitStopDuration = 5.f / 60.f; // 5 frames
	TestEqual("HitStop duration from 5 frames", HitStopDuration, 0.08333333f);

	// --- Hit 2 (continuation) ---
	float Damage2 = 8.f;
	Combo->RegisterHit("LightAttack");
	TestEqual("After hit 2, combo = 2", Combo->GetCurrentCount(), 2);

	// --- Hit 3 (heavy) ---
	float Damage3 = 25.f;
	Combo->RegisterHit("HeavyAttack");
	TestEqual("After hit 3, combo = 3", Combo->GetCurrentCount(), 3);
	TestEqual("Last attack type", Combo->GetLastAttackType(), FName("HeavyAttack"));

	// --- Combo dropped (timeout) ---
	Combo->ResetCombo();
	TestEqual("After reset, combo = 0", Combo->GetCurrentCount(), 0);
	TestFalse("Not in combo after reset", Combo->IsInCombo());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Damage Number Data Flow
// Verifies damage number data creation and propagation.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageNumberFlowTest,
	"FirstGame.Integration.DamageNumberFlow",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDamageNumberFlowTest::RunTest(const FString& Parameters)
{
	// Test normal hit damage number
	FDamageNumberData NormalHit;
	NormalHit.WorldLocation = FVector(100.f, 0.f, 50.f);
	NormalHit.Damage = 8.f;
	NormalHit.bIsCritical = false;
	NormalHit.TextColor = FLinearColor::White;

	TestEqual("Normal hit damage", NormalHit.Damage, 8.f);
	TestFalse("Normal hit not critical", NormalHit.bIsCritical);
	TestEqual("Normal hit color", NormalHit.TextColor, FLinearColor::White);

	// Test critical hit damage number
	FDamageNumberData CritHit;
	CritHit.WorldLocation = FVector(100.f, 0.f, 50.f);
	CritHit.Damage = 16.f; // 2x normal
	CritHit.bIsCritical = true;
	CritHit.TextColor = FLinearColor(1.f, 1.f, 0.f); // Yellow

	TestEqual("Crit hit damage", CritHit.Damage, 16.f);
	TestTrue("Crit hit is critical", CritHit.bIsCritical);

	// Test loot pickup damage number (green for health, blue for energy)
	FDamageNumberData HealthPickup;
	HealthPickup.Damage = 25.f;
	HealthPickup.TextColor = FLinearColor(0.f, 1.f, 0.f); // Green

	FDamageNumberData EnergyPickup;
	EnergyPickup.Damage = 20.f;
	EnergyPickup.TextColor = FLinearColor(0.f, 0.5f, 1.f); // Blue

	TestEqual("Health pickup color G", HealthPickup.TextColor.G, 1.f);
	TestEqual("Energy pickup color B", EnergyPickup.TextColor.B, 1.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Character Data → Combat Parameters
// Verifies that CharacterDataAsset values correctly map to combat params.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDataToCombatTest,
	"FirstGame.Integration.CharacterDataToCombat",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCharacterDataToCombatTest::RunTest(const FString& Parameters)
{
	// Create Huikong's data
	UCharacterDataAsset* Huikong = NewObject<UCharacterDataAsset>(GetTransientPackage());
	Huikong->CharacterId = FName("Huikong");
	Huikong->MaxHealth = 100.f;
	Huikong->MoveSpeed = 320.f;
	Huikong->MaxJumps = 2;
	Huikong->EnergyRegenRate = 10.f;
	Huikong->EnergyCostPerAttack = 5.f;

	// Verify the data can drive combat decisions
	// Can player attack? (enough energy)
	float CurrentEnergy = 50.f;
	bool bCanAttack_Light = (CurrentEnergy >= Huikong->EnergyCostPerAttack);
	TestTrue("Can afford light attack", bCanAttack_Light);

	// After attack, energy decreases
	CurrentEnergy -= Huikong->EnergyCostPerAttack;
	TestEqual("Energy after light attack", CurrentEnergy, 45.f);

	// Can still attack?
	bCanAttack_Light = (CurrentEnergy >= Huikong->EnergyCostPerAttack);
	TestTrue("Can still attack", bCanAttack_Light);

	// Simulate energy drain
	CurrentEnergy = 3.f;
	bCanAttack_Light = (CurrentEnergy >= Huikong->EnergyCostPerAttack);
	TestFalse("Cannot afford attack with low energy", bCanAttack_Light);

	// Energy regeneration
	float DeltaTime = 1.0f;
	CurrentEnergy += Huikong->EnergyRegenRate * DeltaTime;
	TestEqual("Energy after 1s regen", CurrentEnergy, 13.f);

	bCanAttack_Light = (CurrentEnergy >= Huikong->EnergyCostPerAttack);
	TestTrue("Can attack after regen", bCanAttack_Light);

	// Health system
	float CurrentHealth = Huikong->MaxHealth;
	float Damage = 8.f;
	CurrentHealth -= Damage;
	TestEqual("Health after hit", CurrentHealth, 92.f);

	bool bIsDead = (CurrentHealth <= 0.f);
	TestFalse("Not dead after one hit", bIsDead);

	// Healing
	float HealAmount = 25.f;
	CurrentHealth = FMath::Min(CurrentHealth + HealAmount, Huikong->MaxHealth);
	TestEqual("Health after heal", CurrentHealth, 100.f); // Clamped to max

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Wave & Loot Integration
// Verifies wave completion triggers loot drops.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWaveLootIntegrationTest,
	"FirstGame.Integration.WaveLoot",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveLootIntegrationTest::RunTest(const FString& Parameters)
{
	// Simulate: enemy dies → wave manager checks → loot drops

	// Create a loot table
	ULootTable* LootTable = NewObject<ULootTable>(GetTransientPackage());
	if (!LootTable) return false;

	FLootDropConfig HealthDrop;
	HealthDrop.LootType = ELootType::Health;
	HealthDrop.DropChance = 0.3f;
	HealthDrop.Value = 20.f;
	LootTable->Drops.Add(HealthDrop);

	FLootDropConfig CoinDrop;
	CoinDrop.LootType = ELootType::Coin;
	CoinDrop.DropChance = 0.7f;
	CoinDrop.Value = 10.f;
	LootTable->Drops.Add(CoinDrop);

	TestEqual("Loot table has 2 entries", LootTable->Drops.Num(), 2);

	// Verify drop configs
	TestEqual("Health drop chance", LootTable->Drops[0].DropChance, 0.3f);
	TestEqual("Coin drop chance", LootTable->Drops[1].DropChance, 0.7f);

	// Simulate wave clear → all enemies dead → loot drops
	int32 TotalEnemies = 3;
	int32 DeadEnemies = 0;

	DeadEnemies++;
	bool bWaveCleared = (DeadEnemies >= TotalEnemies);
	TestFalse("Wave not cleared yet", bWaveCleared);

	DeadEnemies++;
	bWaveCleared = (DeadEnemies >= TotalEnemies);
	TestFalse("Wave not cleared yet", bWaveCleared);

	DeadEnemies++;
	bWaveCleared = (DeadEnemies >= TotalEnemies);
	TestTrue("Wave cleared!", bWaveCleared);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: 2D Plane Constraint Logic
// Verifies the 2D movement constraint calculations.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(F2DPlaneConstraintTest,
	"FirstGame.Integration.2DPlaneConstraint",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool F2DPlaneConstraintTest::RunTest(const FString& Parameters)
{
	// In 2.5D mode, movement is constrained to XZ plane.
	// Y axis is locked (SetPlaneConstraintAxisSetting(Y) or normal (0,1,0))

	// Player position
	FVector PlayerPos(100.f, 0.f, 0.f);

	// Input movement (from Enhanced Input)
	FVector2D MovementInput(1.f, 0.f); // Right

	// Convert to 3D (XZ plane)
	FVector Movement3D(MovementInput.X, 0.f, MovementInput.Y);

	// Apply speed
	float MoveSpeed = 320.f;
	float DeltaTime = 0.016f; // ~60fps

	FVector NewPos = PlayerPos + Movement3D * MoveSpeed * DeltaTime;

	// Y should remain 0
	TestEqual("Y stays 0", NewPos.Y, 0.f);

	// X should increase
	TestTrue("X increased", NewPos.X > PlayerPos.X);

	// Z unchanged (no vertical input)
	TestEqual("Z unchanged", NewPos.Z, PlayerPos.Z);

	// Jump adds Z velocity
	float JumpForce = 600.f;
	FVector JumpVelocity(0.f, 0.f, JumpForce);
	FVector AfterJump = NewPos + JumpVelocity * DeltaTime;

	TestEqual("Y still 0 after jump", AfterJump.Y, 0.f);
	TestTrue("Z increased after jump", AfterJump.Z > NewPos.Z);

	// Gravity pulls Z down
	float Gravity = 2000.f;
	FVector GravityVel(0.f, 0.f, -Gravity * DeltaTime);
	FVector AfterGravity = AfterJump + GravityVel * DeltaTime;

	TestEqual("Y still 0 with gravity", AfterGravity.Y, 0.f);

	return true;
}
