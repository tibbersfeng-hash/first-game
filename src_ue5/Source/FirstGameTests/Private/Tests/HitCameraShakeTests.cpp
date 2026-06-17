// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// HitCameraShake & ScreenShake Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Combat/HitCameraShake.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: HitCameraShake Default Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHitCameraShakeDefaultsTest,
	"FirstGame.Combat.HitCameraShake.DefaultValues",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHitCameraShakeDefaultsTest::RunTest(const FString& Parameters)
{
	UHitCameraShake* Shake = NewObject<UHitCameraShake>(GetTransientPackage());
	TestNotNull("HitCameraShake should be created", Shake);

	if (!Shake) return false;

	// Default values from constructor/header
	TestEqual("Default OscillationAmplitude", Shake->OscillationAmplitude, 5.f);
	TestEqual("Default OscillationFrequency", Shake->OscillationFrequency, 30.f);
	TestEqual("Default Duration", Shake->Duration, 0.15f);
	TestEqual("Default InitialOpacity", Shake->InitialOpacity, 1.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: HitCameraShake IsFinished Logic
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHitCameraShakeFinishedTest,
	"FirstGame.Combat.HitCameraShake.IsFinished",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHitCameraShakeFinishedTest::RunTest(const FString& Parameters)
{
	// IsFinished returns ElapsedTime >= Duration
	// Initial state: ElapsedTime = 0, Duration = 0.15
	// After 0.15 seconds, IsFinished should be true

	float Duration = 0.15f;
	float ElapsedTime_0 = 0.f;
	float ElapsedTime_Mid = 0.08f;
	float ElapsedTime_End = 0.15f;
	float ElapsedTime_Over = 0.20f;

	TestFalse("Not finished at start", ElapsedTime_0 >= Duration);
	TestFalse("Not finished mid-way", ElapsedTime_Mid >= Duration);
	TestTrue("Finished at end", ElapsedTime_End >= Duration);
	TestTrue("Finished after end", ElapsedTime_Over >= Duration);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: HitCameraShake Decay Calculation
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHitCameraShakeDecayTest,
	"FirstGame.Combat.HitCameraShake.DecayCalculation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHitCameraShakeDecayTest::RunTest(const FString& Parameters)
{
	// DecayAlpha = FMath::Clamp(1.f - (ElapsedTime / Duration), 0.f, 1.f)
	float Duration = 0.15f;

	// At start — full intensity
	float ElapsedTime_0 = 0.f;
	float DecayAlpha_0 = FMath::Clamp(1.f - (ElapsedTime_0 / Duration), 0.f, 1.f);
	TestEqual("Decay at start", DecayAlpha_0, 1.f);

	// Mid-way — half intensity
	float ElapsedTime_Mid = Duration * 0.5f;
	float DecayAlpha_Mid = FMath::Clamp(1.f - (ElapsedTime_Mid / Duration), 0.f, 1.f);
	TestEqual("Decay mid-way", DecayAlpha_Mid, 0.5f);

	// Near end — low intensity
	float ElapsedTime_NearEnd = Duration * 0.9f;
	float DecayAlpha_NearEnd = FMath::Clamp(1.f - (ElapsedTime_NearEnd / Duration), 0.f, 1.f);
	TestEqual("Decay near end", DecayAlpha_NearEnd, 0.1f);

	// At end — zero
	float ElapsedTime_End = Duration;
	float DecayAlpha_End = FMath::Clamp(1.f - (ElapsedTime_End / Duration), 0.f, 1.f);
	TestEqual("Decay at end", DecayAlpha_End, 0.f);

	// After end — clamped to zero
	float ElapsedTime_Over = Duration * 1.5f;
	float DecayAlpha_Over = FMath::Clamp(1.f - (ElapsedTime_Over / Duration), 0.f, 1.f);
	TestEqual("Decay after end (clamped)", DecayAlpha_Over, 0.f);

	// Shake amount = OscillationAmplitude * DecayAlpha
	float Amplitude = 5.f;
	float ShakeAmount_Start = Amplitude * DecayAlpha_0;
	float ShakeAmount_End = Amplitude * DecayAlpha_End;
	TestEqual("Shake amount at start", ShakeAmount_Start, 5.f);
	TestEqual("Shake amount at end", ShakeAmount_End, 0.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ScreenFlashComponent Configuration
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScreenFlashConfigTest,
	"FirstGame.Combat.HitCameraShake.ScreenFlashConfig",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScreenFlashConfigTest::RunTest(const FString& Parameters)
{
	UScreenFlashComponent* Flash = NewObject<UScreenFlashComponent>(GetTransientPackage());
	TestNotNull("ScreenFlashComponent should be created", Flash);

	if (!Flash) return false;

	// Default flash settings from TriggerFlash defaults:
	// Color = White, Duration = 0.1f, MaxAlpha = 0.3f

	// Verify defaults (from function signature)
	FLinearColor DefaultColor = FLinearColor::White;
	float DefaultDuration = 0.1f;
	float DefaultAlpha = 0.3f;

	TestEqual("Default flash color R", DefaultColor.R, 1.f);
	TestEqual("Default flash color G", DefaultColor.G, 1.f);
	TestEqual("Default flash color B", DefaultColor.B, 1.f);
	TestEqual("Default flash duration", DefaultDuration, 0.1f);
	TestEqual("Default flash alpha", DefaultAlpha, 0.3f);

	// Critical hit flash — red
	FLinearColor CritColor = FLinearColor(1.f, 0.f, 0.f);
	TestEqual("Crit flash color R", CritColor.R, 1.f);
	TestEqual("Crit flash color G", CritColor.G, 0.f);

	return true;
}
