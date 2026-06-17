// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// ComboManager Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Combat/ComboManager.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager Initialization
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerInitTest,
	"FirstGame.Combat.ComboManager.Initialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FComboManagerInitTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	TestNotNull("ComboManager should be created", Combo);

	if (!Combo) return false;

	// Default combo count should be 0
	TestEqual("Initial count should be 0", Combo->GetCurrentCount(), 0);

	// Should not be in combo initially
	TestFalse("Should not be in combo initially", Combo->IsInCombo());

	// Default combo window time
	TestEqual("Default combo window", Combo->ComboWindowTime, 1.5f);

	// Default max combo count
	TestEqual("Default max combo", Combo->MaxComboCount, 99);

	// Default attack type should be NAME_None
	TestEqual("Initial attack type", Combo->GetLastAttackType(), NAME_None);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager First Hit
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerFirstHitTest,
	"FirstGame.Combat.ComboManager.FirstHit",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FComboManagerFirstHitTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	if (!Combo) return false;

	// Register first hit
	Combo->RegisterHit("LightAttack");

	// Count should be 1
	TestEqual("Count after first hit", Combo->GetCurrentCount(), 1);

	// Should be in combo
	TestTrue("Should be in combo after first hit", Combo->IsInCombo());

	// Attack type should be set
	TestEqual("Attack type after first hit", Combo->GetLastAttackType(), FName("LightAttack"));

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager Combo Continuation
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerContinuationTest,
	"FirstGame.Combat.ComboManager.ComboContinuation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FComboManagerContinuationTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	if (!Combo) return false;

	// Register multiple hits in sequence
	Combo->RegisterHit("LightAttack");
	Combo->RegisterHit("LightAttack");
	Combo->RegisterHit("HeavyAttack");

	// Count should be 3
	TestEqual("Count after 3 hits", Combo->GetCurrentCount(), 3);

	// Last attack type should be HeavyAttack
	TestEqual("Last attack type", Combo->GetLastAttackType(), FName("HeavyAttack"));

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager Max Combo Clamp
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerMaxClampTest,
	"FirstGame.Combat.ComboManager.MaxComboClamp",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FComboManagerMaxClampTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	if (!Combo) return false;

	// Set a low max combo for testing
	Combo->MaxComboCount = 5;

	// Register more hits than max
	for (int32 i = 0; i < 10; i++)
	{
		Combo->RegisterHit("LightAttack");
	}

	// Count should be clamped to 5
	TestEqual("Count should be clamped to max", Combo->GetCurrentCount(), 5);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager ResetCombo
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerResetTest,
	"FirstGame.Combat.ComboManager.ResetCombo",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FComboManagerResetTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	if (!Combo) return false;

	// Build up a combo
	Combo->RegisterHit("LightAttack");
	Combo->RegisterHit("HeavyAttack");
	TestEqual("Combo count before reset", Combo->GetCurrentCount(), 2);

	// Reset
	Combo->ResetCombo();

	// Should be back to zero
	TestEqual("Count after reset", Combo->GetCurrentCount(), 0);
	TestFalse("Should not be in combo after reset", Combo->IsInCombo());
	TestEqual("Attack type after reset", Combo->GetLastAttackType(), NAME_None);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager GetNextComboIndex
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerNextIndexTest,
	"FirstGame.Combat.ComboManager.GetNextComboIndex",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FComboManagerNextIndexTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	if (!Combo) return false;

	// Initial state — no combo, index should be 0
	TestEqual("Next index at start", Combo->GetNextComboIndex(), 0);

	// After first hit — index should be 1
	Combo->RegisterHit("LightAttack");
	TestEqual("Next index after 1 hit", Combo->GetNextComboIndex(), 1);

	// After second hit — index should be 2
	Combo->RegisterHit("LightAttack");
	TestEqual("Next index after 2 hits", Combo->GetNextComboIndex(), 2);

	// After third hit — index should still be 2 (clamped at 2)
	Combo->RegisterHit("LightAttack");
	TestEqual("Next index after 3 hits (clamped)", Combo->GetNextComboIndex(), 2);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager Window Configuration
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerWindowConfigTest,
	"FirstGame.Combat.ComboManager.WindowConfiguration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FComboManagerWindowConfigTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	if (!Combo) return false;

	// Verify combo window is configurable
	Combo->ComboWindowTime = 2.5f;
	TestEqual("Custom combo window", Combo->ComboWindowTime, 2.5f);

	// Verify max combo is configurable
	Combo->MaxComboCount = 10;
	TestEqual("Custom max combo", Combo->MaxComboCount, 10);

	// Register hit and verify it works with custom settings
	Combo->RegisterHit("LightAttack");
	TestEqual("Count with custom settings", Combo->GetCurrentCount(), 1);

	return true;
}
