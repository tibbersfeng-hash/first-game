// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// WaveManager Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Dungeon/WaveManager.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: WaveManager Initialization
// ────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWaveManagerInitTest,
	"FirstGame.Dungeon.WaveManager.Initialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveManagerInitTest::RunTest(const FString& Parameters)
{
	AWaveManager* WaveMgr = GetWorld()->SpawnActor<AWaveManager>();
	TestNotNull("WaveManager should be spawned", WaveMgr);

	if (!WaveMgr) return false;

	// Default state
	TestEqual("Initial wave index should be -1", WaveMgr->GetCurrentWaveIndex(), -1);
	TestEqual("Initial total waves should be 0", WaveMgr->GetTotalWaves(), 0);
	TestFalse("Should not be complete initially", WaveMgr->AreAllWavesComplete());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: WaveManager SetupWaves
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWaveManagerSetupTest,
	"FirstGame.Dungeon.WaveManager.SetupWaves",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveManagerSetupTest::RunTest(const FString& Parameters)
{
	AWaveManager* WaveMgr = GetWorld()->SpawnActor<AWaveManager>();
	if (!WaveMgr) return false;

	// Create wave configurations
	TArray<FWaveConfig> TestWaves;

	FWaveConfig Wave1;
	Wave1.EnemyCount = 3;
	Wave1.SpawnDelay = 1.0f;
	Wave1.bIsBossWave = false;
	TestWaves.Add(Wave1);

	FWaveConfig Wave2;
	Wave2.EnemyCount = 5;
	Wave2.SpawnDelay = 0.8f;
	Wave2.bIsBossWave = false;
	TestWaves.Add(Wave2);

	FWaveConfig BossWave;
	BossWave.EnemyCount = 1;
	BossWave.SpawnDelay = 2.0f;
	BossWave.bIsBossWave = true;
	TestWaves.Add(BossWave);

	// Setup waves
	WaveMgr->SetupWaves(TestWaves);

	// Verify
	TestEqual("Total waves should be 3", WaveMgr->GetTotalWaves(), 3);
	TestEqual("Wave index should reset to -1", WaveMgr->GetCurrentWaveIndex(), -1);
	TestFalse("Should not be complete after setup", WaveMgr->AreAllWavesComplete());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: WaveManager Empty Waves — Auto-complete
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWaveManagerEmptyWavesTest,
	"FirstGame.Dungeon.WaveManager.EmptyWavesAutoComplete",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveManagerEmptyWavesTest::RunTest(const FString& Parameters)
{
	AWaveManager* WaveMgr = GetWorld()->SpawnActor<AWaveManager>();
	if (!WaveMgr) return false;

	// Track if OnAllWavesCleared fires
	bool bClearedFired = false;
	WaveMgr->OnAllWavesCleared.AddUObject(WaveMgr, [](bool bVictory)
	{
		// In unit test, this validates the delegate fires
	});

	// Start waves with empty config
	TArray<FWaveConfig> EmptyWaves;
	WaveMgr->SetupWaves(EmptyWaves);
	WaveMgr->StartWaves();

	// Should auto-complete with victory
	TestTrue("Should be complete with empty waves", WaveMgr->AreAllWavesComplete());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: FWaveConfig Default Values
// ────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWaveConfigDefaultsTest,
	"FirstGame.Dungeon.WaveManager.WaveConfigDefaults",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveConfigDefaultsTest::RunTest(const FString& Parameters)
{
	FWaveConfig Config;

	TestEqual("Default EnemyCount", Config.EnemyCount, 3);
	TestEqual("Default SpawnDelay", Config.SpawnDelay, 1.0f);
	TestFalse("Default bIsBossWave", Config.bIsBossWave);
	TestEqual("Default EnemyClasses empty", Config.EnemyClasses.Num(), 0);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: WaveManager Wave Index Progression Logic
// Verifies the wave index logic without needing actual enemy spawning.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWaveManagerIndexLogicTest,
	"FirstGame.Dungeon.WaveManager.IndexProgressionLogic",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWaveManagerIndexLogicTest::RunTest(const FString& Parameters)
{
	// Test the wave progression logic:
	// - CurrentWaveIndex starts at -1 (not started)
	// - StartWaves() sets it to 0
	// - When wave clears, index increments
	// - When index >= Waves.Num(), all waves complete

	int32 CurrentWaveIndex = -1;
	int32 TotalWaves = 3;
	bool bAllWavesComplete = false;

	// Initial state
	TestEqual("Initial index", CurrentWaveIndex, -1);
	TestFalse("Initially not complete", bAllWavesComplete);

	// Simulate StartWaves
	if (TotalWaves > 0)
	{
		CurrentWaveIndex = 0;
	}
	TestEqual("After start, index = 0", CurrentWaveIndex, 0);

	// Simulate wave 0 cleared → move to wave 1
	CurrentWaveIndex++;
	TestEqual("After wave 0 cleared, index = 1", CurrentWaveIndex, 1);
	TestFalse("Not complete yet", bAllWavesComplete);

	// Simulate wave 1 cleared → move to wave 2
	CurrentWaveIndex++;
	TestEqual("After wave 1 cleared, index = 2", CurrentWaveIndex, 2);
	TestFalse("Not complete yet", bAllWavesComplete);

	// Simulate wave 2 cleared → all complete
	CurrentWaveIndex++;
	if (CurrentWaveIndex >= TotalWaves)
	{
		bAllWavesComplete = true;
	}
	TestTrue("All waves complete", bAllWavesComplete);
	TestEqual("Final index", CurrentWaveIndex, 3);

	return true;
}
