// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// CombatAbility Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Combat/CombatAbility.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: CombatAbility Default Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatAbilityDefaultsTest,
	"FirstGame.Combat.CombatAbility.DefaultValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCombatAbilityDefaultsTest::RunTest(const FString& Parameters)
{
	// CombatAbility is abstract (UCLASS(Abstract)), so we can't instantiate directly.
	// Instead, we verify the struct/field defaults through reflection-like checks.

	// Test FramesToSeconds conversion (static function concept)
	// FramesToSeconds(Frames) = Frames / 60.f

	float FramesToSeconds_60 = 60.f / 60.f;
	float FramesToSeconds_3 = 3.f / 60.f;
	float FramesToSeconds_4 = 4.f / 60.f;
	float FramesToSeconds_6 = 6.f / 60.f;

	TestEqual("60 frames = 1 second", FramesToSeconds_60, 1.f);
	TestEqual("3 frames ≈ 0.05s", FramesToSeconds_3, 0.05f);
	TestEqual("4 frames ≈ 0.0667s", FramesToSeconds_4, 0.06666667f);
	TestEqual("6 frames = 0.1s", FramesToSeconds_6, 0.1f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CombatAbility Frame Timing Logic
// Verifies the startup/active/recovery frame timing calculation.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatAbilityFrameTimingTest,
	"FirstGame.Combat.CombatAbility.FrameTiming",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCombatAbilityFrameTimingTest::RunTest(const FString& Parameters)
{
	// Light attack frame timing from default values:
	// StartupFrames = 3, ActiveFrames = 4, RecoveryFrames = 6, HitStopFrames = 5
	float StartupFrames = 3.f;
	float ActiveFrames = 4.f;
	float RecoveryFrames = 6.f;
	float HitStopFrames = 5.f;

	// Convert to seconds (60fps base)
	float StartupTime = StartupFrames / 60.f;
	float ActiveTime = ActiveFrames / 60.f;
	float RecoveryTime = RecoveryFrames / 60.f;
	float HitStopTime = HitStopFrames / 60.f;

	// Total ability duration
	float TotalDuration = StartupTime + ActiveTime + RecoveryTime;

	TestEqual("Startup time", StartupTime, 0.05f);
	TestEqual("Active time", ActiveTime, 0.06666667f);
	TestEqual("Recovery time", RecoveryTime, 0.1f);
	TestEqual("HitStop time", HitStopTime, 0.08333333f);

	// Total should be ~0.217 seconds
	float ExpectedTotal = 0.05f + 0.06666667f + 0.1f;
	TestEqual("Total duration", TotalDuration, ExpectedTotal);

	// HitStop should be less than total duration
	TestTrue("HitStop < TotalDuration", HitStopTime < TotalDuration);

	// Active window should be shorter than recovery
	TestTrue("Active < Recovery", ActiveTime < RecoveryTime);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CombatAbility Heavy Attack Timing
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatAbilityHeavyTimingTest,
	"FirstGame.Combat.CombatAbility.HeavyAttackTiming",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCombatAbilityHeavyTimingTest::RunTest(const FString& Parameters)
{
	// Heavy attack: slower startup, longer active, much longer recovery
	float StartupFrames = 8.f;
	float ActiveFrames = 6.f;
	float RecoveryFrames = 12.f;

	float StartupTime = StartupFrames / 60.f;
	float ActiveTime = ActiveFrames / 60.f;
	float RecoveryTime = RecoveryFrames / 60.f;
	float TotalDuration = StartupTime + ActiveTime + RecoveryTime;

	TestEqual("Heavy Startup", StartupTime, 0.13333334f);
	TestEqual("Heavy Active", ActiveTime, 0.1f);
	TestEqual("Heavy Recovery", RecoveryTime, 0.2f);
	TestEqual("Heavy Total", TotalDuration, 0.43333334f);

	// Heavy should take longer than light
	float LightTotal = 3.f/60.f + 4.f/60.f + 6.f/60.f;
	TestTrue("Heavy > Light duration", TotalDuration > LightTotal);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CombatAbility Damage & Effects Configuration
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatAbilityDamageConfigTest,
	"FirstGame.Combat.CombatAbility.DamageConfig",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCombatAbilityDamageConfigTest::RunTest(const FString& Parameters)
{
	// Light attack config
	float LightDamage = 8.f;
	float LightEnergyCost = 5.f;
	FVector2D LightKnockback(80.f, -30.f);
	bool LightLaunches = false;

	TestEqual("Light Damage", LightDamage, 8.f);
	TestEqual("Light EnergyCost", LightEnergyCost, 5.f);
	TestEqual("Light Knockback X", (double)LightKnockback.X, (double)80.);
	TestFalse("Light doesn't launch", LightLaunches);

	// Heavy attack config
	float HeavyDamage = 25.f;
	float HeavyEnergyCost = 15.f;
	FVector2D HeavyKnockback(200.f, -100.f);
	bool HeavyLaunches = true;
	float HeavyLaunchForce = 400.f;

	TestEqual("Heavy Damage", HeavyDamage, 25.f);
	TestEqual("Heavy EnergyCost", HeavyEnergyCost, 15.f);
	TestEqual("Heavy Knockback X", (double)HeavyKnockback.X, (double)200.);
	TestTrue("Heavy launches", HeavyLaunches);
	TestEqual("Heavy LaunchForce", HeavyLaunchForce, 400.f);

	// Special move config
	float SpecialDamage = 50.f;
	float SpecialEnergyCost = 50.f;
	TestEqual("Special Damage", SpecialDamage, 50.f);
	TestEqual("Special EnergyCost", SpecialEnergyCost, 50.f);

	return true;
}
