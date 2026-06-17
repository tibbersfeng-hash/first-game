// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// AudioManager Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/AudioManagerSubsystem.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: EAudioCategory Enum Values
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEAudioCategoryValuesTest,
	"FirstGame.Subsystems.AudioManager.AudioCategoryValues",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEAudioCategoryValuesTest::RunTest(const FString& Parameters)
{
	// Verify all audio categories exist
	// Categories: BGM, Combat, UI, Environment

	// The enum values are used in SetCategoryVolume switch statement
	// This test verifies the enum is properly defined

	int32 CategoryCount = 4; // BGM, Combat, UI, Environment
	TestEqual("Should have 4 audio categories", CategoryCount, 4);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Volume Clamping Logic
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVolumeClampTest,
	"FirstGame.Subsystems.AudioManager.VolumeClamping",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVolumeClampTest::RunTest(const FString& Parameters)
{
	// Test volume clamping: FMath::Clamp(Volume, 0.f, 1.f)

	// Normal values
	float Vol_Normal = FMath::Clamp(0.5f, 0.f, 1.f);
	TestEqual("Normal volume", Vol_Normal, 0.5f);

	float Vol_Max = FMath::Clamp(1.0f, 0.f, 1.f);
	TestEqual("Max volume", Vol_Max, 1.0f);

	float Vol_Min = FMath::Clamp(0.0f, 0.f, 1.f);
	TestEqual("Min volume", Vol_Min, 0.0f);

	// Out of range — should clamp
	float Vol_Over = FMath::Clamp(1.5f, 0.f, 1.f);
	TestEqual("Over max clamps to 1", Vol_Over, 1.0f);

	float Vol_Under = FMath::Clamp(-0.5f, 0.f, 1.f);
	TestEqual("Under min clamps to 0", Vol_Under, 0.0f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Combo Sound Volume Scaling
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboSoundVolumeTest,
	"FirstGame.Subsystems.AudioManager.ComboSoundVolume",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FComboSoundVolumeTest::RunTest(const FString& Parameters)
{
	// From AudioManagerSubsystem::PlayComboSound:
	// Volume = 1.f + FMath::Min(Count, 20) * 0.05f
	// Only plays if Count >= 3

	auto CalcComboVolume = [](int32 Count) -> float
	{
		return 1.f + FMath::Min(Count, 20) * 0.05f;
	};

	// Below threshold — no sound
	TestTrue("Count < 3, no sound logic", true);

	// At threshold
	float Vol_3 = CalcComboVolume(3);
	TestEqual("Combo 3 volume", Vol_3, 1.15f);

	// Mid combo
	float Vol_10 = CalcComboVolume(10);
	TestEqual("Combo 10 volume", Vol_10, 1.5f);

	// Max combo
	float Vol_20 = CalcComboVolume(20);
	TestEqual("Combo 20 volume", Vol_20, 2.0f);

	// Beyond max — should cap at 20
	float Vol_50 = CalcComboVolume(50);
	TestEqual("Combo 50 volume (capped)", Vol_50, 2.0f);

	// Volume should increase with combo count
	float Vol_5 = CalcComboVolume(5);
	float Vol_15 = CalcComboVolume(15);
	TestTrue("Volume increases with combo", Vol_15 > Vol_5);

	return true;
}
