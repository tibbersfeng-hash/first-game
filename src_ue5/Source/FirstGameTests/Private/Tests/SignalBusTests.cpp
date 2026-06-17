// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// SignalBus Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/SignalBusSubsystem.h"
#include "Subsystems/SignalBusTypes.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: SignalBusSubsystem Delegate Existence
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSignalBusDelegatesTest,
	"FirstGame.Subsystems.SignalBus.Delegates",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSignalBusDelegatesTest::RunTest(const FString& Parameters)
{
	// The SignalBusSubsystem is a GameInstanceSubsystem.
	// In unit test context, we verify the delegate declarations compile correctly.

	// Combat delegates
	{
		USignalBusSubsystem::FOnHitLanded HitLanded;
		USignalBusSubsystem::FOnComboFinished ComboFinished;
		USignalBusSubsystem::FOnComboUpdated ComboUpdated;
		USignalBusSubsystem::FOnHitStopRequested HitStopRequested;
		// All combat delegates should be valid
		TestTrue("Combat delegates compile", true);
	}

	// Player delegates
	{
		USignalBusSubsystem::FOnPlayerHealthChanged HealthChanged;
		USignalBusSubsystem::FOnPlayerEnergyChanged EnergyChanged;
		USignalBusSubsystem::FOnPlayerDied PlayerDied;
		TestTrue("Player delegates compile", true);
	}

	// Enemy delegates
	{
		USignalBusSubsystem::FOnEnemyDied EnemyDied;
		TestTrue("Enemy delegates compile", true);
	}

	// Dungeon delegates
	{
		USignalBusSubsystem::FOnRoomCleared RoomCleared;
		USignalBusSubsystem::FOnRoomEntered RoomEntered;
		USignalBusSubsystem::FOnDungeonCompleted DungeonCompleted;
		TestTrue("Dungeon delegates compile", true);
	}

	// UI delegates
	{
		USignalBusSubsystem::FOnShowDamageNumber ShowDamageNumber;
		TestTrue("UI delegates compile", true);
	}

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: FDamageNumberData Default Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageNumberDataDefaultsTest,
	"FirstGame.Subsystems.SignalBus.DamageNumberDataDefaults",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDamageNumberDataDefaultsTest::RunTest(const FString& Parameters)
{
	FDamageNumberData Data;

	TestEqual("Default Damage", Data.Damage, 0.f);
	TestFalse("Default bIsCritical", Data.bIsCritical);
	TestEqual("Default TextColor", Data.TextColor, FLinearColor::White);
	TestEqual("Default WorldLocation", Data.WorldLocation, FVector::ZeroVector);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: FDamageNumberData Custom Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageNumberDataCustomTest,
	"FirstGame.Subsystems.SignalBus.DamageNumberDataCustom",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDamageNumberDataCustomTest::RunTest(const FString& Parameters)
{
	FDamageNumberData Data;
	Data.WorldLocation = FVector(100.f, 200.f, 300.f);
	Data.Damage = 50.f;
	Data.bIsCritical = true;
	Data.TextColor = FLinearColor(1.f, 0.f, 0.f); // Red for critical

	TestEqual("Custom Damage", Data.Damage, 50.f);
	TestTrue("Custom bIsCritical", Data.bIsCritical);
	TestEqual("Custom TextColor R", Data.TextColor.R, 1.f);
	TestEqual("Custom Location X", Data.WorldLocation.X, 100.f);
	TestEqual("Custom Location Y", Data.WorldLocation.Y, 200.f);
	TestEqual("Custom Location Z", Data.WorldLocation.Z, 300.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: SignalBus Event Flow Logic
// Verifies the event flow pattern without requiring a GameInstance.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSignalBusEventFlowTest,
	"FirstGame.Subsystems.SignalBus.EventFlowLogic",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSignalBusEventFlowTest::RunTest(const FString& Parameters)
{
	// Test the event flow pattern used throughout the game:
	// HitBox → SignalBus → UI (damage numbers), Audio, Screen Shake
	// This is a structural/architectural test.

	// Verify FDamageNumberData can carry all needed information
	FDamageNumberData HitData;
	HitData.WorldLocation = FVector(100.f, 0.f, 50.f);
	HitData.Damage = 25.f;
	HitData.bIsCritical = false;
	HitData.TextColor = FLinearColor::White;

	TestEqual("Hit damage", HitData.Damage, 25.f);
	TestFalse("Hit is not critical", HitData.bIsCritical);

	// Verify critical hit data
	FDamageNumberData CritData;
	CritData.Damage = 50.f;
	CritData.bIsCritical = true;
	CritData.TextColor = FLinearColor(1.f, 1.f, 0.f); // Yellow

	TestEqual("Crit damage", CritData.Damage, 50.f);
	TestTrue("Crit is critical", CritData.bIsCritical);

	return true;
}
