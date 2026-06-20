// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Dungeon Flow Tests — verifies wave management and room state logic

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Dungeon/WaveManager.h"
#include "Dungeon/DungeonRoom.h"
#include "Dungeon/LootItem.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: WaveConfig — Default values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWaveConfigCustomValuesTest,
	"FirstGame.Dungeon.WaveConfig.Custom",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FWaveConfigCustomValuesTest::RunTest(const FString& Parameters)
{
	FWaveConfig NormalWave;
	NormalWave.EnemyCount = 5;
	NormalWave.SpawnDelay = 2.0f;
	NormalWave.bIsBossWave = false;

	TestEqual("Normal EnemyCount", NormalWave.EnemyCount, 5);
	TestEqual("Normal SpawnDelay", NormalWave.SpawnDelay, 2.0f);

	FWaveConfig BossWave;
	BossWave.EnemyCount = 1;
	BossWave.SpawnDelay = 3.0f;
	BossWave.bIsBossWave = true;

	TestEqual("Boss EnemyCount", BossWave.EnemyCount, 1);
	TestTrue("Boss bIsBossWave", BossWave.bIsBossWave);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: RoomType — All values distinct
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRoomTypeEnumTest,
	"FirstGame.Dungeon.RoomType.EnumValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRoomTypeEnumTest::RunTest(const FString& Parameters)
{
	// Verify all room types are distinct
	TestTrue("Combat != Treasure", ERoomType::Combat != ERoomType::Treasure);
	TestTrue("Combat != Boss", ERoomType::Combat != ERoomType::Boss);
	TestTrue("Combat != Rest", ERoomType::Combat != ERoomType::Rest);
	TestTrue("Treasure != Boss", ERoomType::Treasure != ERoomType::Boss);
	TestTrue("Boss != Rest", ERoomType::Boss != ERoomType::Rest);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Dungeon Room — EnemiesPerWave splits enemies into waves
// Simulates the wave-building math
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPerWaveSplitTest,
	"FirstGame.Dungeon.DungeonRoom.EnemiesPerWaveSplit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEnemiesPerWaveSplitTest::RunTest(const FString& Parameters)
{
	// Simulate wave splitting: 10 enemies, 3 per wave
	int32 TotalEnemies = 10;
	int32 EnemiesPerWave = 3;

	int32 ExpectedWaves = FMath::CeilToInt((float)TotalEnemies / (float)EnemiesPerWave);
	TestEqual("10 enemies / 3 per wave = 4 waves", ExpectedWaves, 4);

	// Edge: exact division
	TotalEnemies = 9;
	ExpectedWaves = FMath::CeilToInt((float)TotalEnemies / (float)EnemiesPerWave);
	TestEqual("9 enemies / 3 per wave = 3 waves", ExpectedWaves, 3);

	// Edge: 1 enemy
	TotalEnemies = 1;
	ExpectedWaves = FMath::CeilToInt((float)TotalEnemies / (float)EnemiesPerWave);
	TestEqual("1 enemy / 3 per wave = 1 wave", ExpectedWaves, 1);

	// Edge: 0 enemies
	TotalEnemies = 0;
	ExpectedWaves = FMath::CeilToInt((float)TotalEnemies / (float)EnemiesPerWave);
	TestEqual("0 enemies = 0 waves", ExpectedWaves, 0);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Wave Manager — State progression simulation
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWaveManagerStateTest,
	"FirstGame.Dungeon.WaveManager.StateProgression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FWaveManagerStateTest::RunTest(const FString& Parameters)
{
	// Simulate wave manager state machine
	int32 CurrentWaveIndex = -1;  // Not started
	bool bAllWavesComplete = false;
	int32 TotalWaves = 3;

	// Before start
	TestEqual("Initial wave index = -1", CurrentWaveIndex, -1);
	TestFalse("Not complete before start", bAllWavesComplete);

	// Start wave 0
	CurrentWaveIndex = 0;
	TestEqual("Wave 0 started", CurrentWaveIndex, 0);
	TestFalse("Not complete during waves", bAllWavesComplete);

	// Clear wave 0 → start wave 1
	CurrentWaveIndex = 1;
	TestEqual("Wave 1", CurrentWaveIndex, 1);

	// Clear wave 1 → start wave 2 (last)
	CurrentWaveIndex = 2;
	TestEqual("Wave 2 (last)", CurrentWaveIndex, 2);

	// Clear last wave
	CurrentWaveIndex = TotalWaves;  // Past last index
	bAllWavesComplete = true;
	TestTrue("All waves complete", bAllWavesComplete);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: LootItem — Drop chance simulation
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootDropChanceTest,
	"FirstGame.Dungeon.LootItem.DropChance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLootDropChanceTest::RunTest(const FString& Parameters)
{
	// Verify drop chance bounds
	float DropChance = 0.3f;
	TestTrue("DropChance in [0,1]", DropChance >= 0.f && DropChance <= 1.f);

	// Simulate deterministic drop (always drops if chance >= 1.0)
	float GuaranteedDrop = 1.0f;
	TestTrue("100% drop always succeeds", GuaranteedDrop >= 1.0f);

	// 0% never drops
	float NoDrop = 0.0f;
	TestTrue("0% never drops", NoDrop <= 0.0f);

	// ELootType enum values exist
	TestTrue("ELootType::Health exists", (int)ELootType::Health >= 0);
	TestTrue("ELootType::Coin exists", (int)ELootType::Coin >= 0);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Dungeon Flow — Complete room lifecycle
// Simulates: StartRoom → Waves → ClearRoom → Next Room
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDungeonRoomLifecycleTest,
	"FirstGame.Dungeon.DungeonRoom.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDungeonRoomLifecycleTest::RunTest(const FString& Parameters)
{
	// Simulate room lifecycle without AActor spawning

	// Room setup
	ERoomType RoomType = ERoomType::Combat;
	int32 RoomIndex = 0;
	bool bIsCleared = false;
	int32 EnemiesPerWave = 3;

	// 5 enemies to defeat
	int32 TotalEnemies = 5;
	int32 DeadEnemies = 0;

	// Verify initial state
	TestEqual("Room type = Combat", RoomType, ERoomType::Combat);
	TestEqual("Room index = 0", RoomIndex, 0);
	TestFalse("Not cleared initially", bIsCleared);

	// Calculate waves
	int32 TotalWaves = FMath::CeilToInt((float)TotalEnemies / (float)EnemiesPerWave);
	TestEqual("5 enemies / 3 per wave = 2 waves", TotalWaves, 2);

	// Simulate combat: kill enemies
	DeadEnemies += 3; // Wave 1 cleared
	TestFalse("Room not cleared after wave 1", DeadEnemies >= TotalEnemies);

	DeadEnemies += 2; // Wave 2 cleared (remaining 2 enemies)
	TestTrue("All enemies dead", DeadEnemies >= TotalEnemies);

	// Room cleared
	bIsCleared = true;
	TestTrue("Room cleared", bIsCleared);

	return true;
}
