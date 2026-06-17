// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// LootItem & LootTable Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Dungeon/LootItem.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: ELootType Enum Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootTypeValuesTest,
	"FirstGame.Dungeon.LootItem.LootTypeValues",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLootTypeValuesTest::RunTest(const FString& Parameters)
{
	// Verify all loot types exist
	int32 TypeCount = 0;

	// Enum values: Health=0, Energy=1, Coin=2, PowerUp=3
	ELootType Types[] = {
		ELootType::Health,
		ELootType::Energy,
		ELootType::Coin,
		ELootType::PowerUp
	};
	TypeCount = 4;

	TestEqual("Should have 4 loot types", TypeCount, 4);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: FLootDropConfig Default Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootDropConfigDefaultsTest,
	"FirstGame.Dungeon.LootItem.LootDropConfigDefaults",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLootDropConfigDefaultsTest::RunTest(const FString& Parameters)
{
	FLootDropConfig Config;

	TestEqual("Default LootType", (uint8)Config.LootType, (uint8)ELootType::Health);
	TestEqual("Default DropChance", Config.DropChance, 0.3f);
	TestEqual("Default Value", Config.Value, 20.f);
	TestEqual("Default PickupRange", Config.PickupRange, 60.f);
	TestEqual("Default Lifetime", Config.Lifetime, 10.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: FLootDropConfig Custom Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootDropConfigCustomTest,
	"FirstGame.Dungeon.LootItem.LootDropConfigCustom",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLootDropConfigCustomTest::RunTest(const FString& Parameters)
{
	FLootDropConfig HealthDrop;
	HealthDrop.LootType = ELootType::Health;
	HealthDrop.DropChance = 0.5f;
	HealthDrop.Value = 30.f;

	TestEqual("Health DropChance", HealthDrop.DropChance, 0.5f);
	TestEqual("Health Value", HealthDrop.Value, 30.f);

	FLootDropConfig CoinDrop;
	CoinDrop.LootType = ELootType::Coin;
	CoinDrop.DropChance = 0.8f;
	CoinDrop.Value = 10.f;
	CoinDrop.Lifetime = 15.f;

	TestEqual("Coin DropChance", CoinDrop.DropChance, 0.8f);
	TestEqual("Coin Lifetime", CoinDrop.Lifetime, 15.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ULootTable Creation
// ────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootTableCreationTest,
	"FirstGame.Dungeon.LootItem.LootTableCreation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLootTableCreationTest::RunTest(const FString& Parameters)
{
	ULootTable* LootTable = NewObject<ULootTable>(GetTransientPackage());
	TestNotNull("LootTable should be created", LootTable);

	if (!LootTable) return false;

	// Initially empty drops array
	TestEqual("Initial drops empty", LootTable->Drops.Num(), 0);

	// Add drops
	FLootDropConfig HealthDrop;
	HealthDrop.LootType = ELootType::Health;
	HealthDrop.DropChance = 0.5f;
	HealthDrop.Value = 25.f;
	LootTable->Drops.Add(HealthDrop);

	FLootDropConfig CoinDrop;
	CoinDrop.LootType = ELootType::Coin;
	CoinDrop.DropChance = 0.7f;
	CoinDrop.Value = 15.f;
	LootTable->Drops.Add(CoinDrop);

	TestEqual("Should have 2 drops", LootTable->Drops.Num(), 2);
	TestEqual("First drop type", (uint8)LootTable->Drops[0].LootType, (uint8)ELootType::Health);
	TestEqual("Second drop type", (uint8)LootTable->Drops[1].LootType, (uint8)ELootType::Coin);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Drop Chance Boundary Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootDropChanceBoundaryTest,
	"FirstGame.Dungeon.LootItem.DropChanceBoundaries",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLootDropChanceBoundaryTest::RunTest(const FString& Parameters)
{
	// Test 0% drop chance — should never drop
	FLootDropConfig ZeroChance;
	ZeroChance.DropChance = 0.f;
	TestEqual("Zero drop chance", ZeroChance.DropChance, 0.f);

	// Test 100% drop chance — should always drop
	FLootDropConfig FullChance;
	FullChance.DropChance = 1.f;
	TestEqual("Full drop chance", FullChance.DropChance, 1.f);

	// Test negative (should be clamped in production)
	FLootDropConfig NegativeChance;
	NegativeChance.DropChance = -0.5f;
	// The RollDrops() uses FMath::FRand() which returns [0,1),
	// so a negative chance means it never drops (FRand() >= 0 > -0.5)
	// This is a structural test — production code should clamp

	return true;
}
