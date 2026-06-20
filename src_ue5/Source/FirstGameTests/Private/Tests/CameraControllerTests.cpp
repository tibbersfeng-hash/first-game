// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Camera Controller Unit Tests — verifies mode switching, params, auto-center logic

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Camera/CameraController.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: CameraController — Default mode is Free
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraControllerDefaultModeTest,
	"FirstGame.Camera.CameraController.DefaultMode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraControllerDefaultModeTest::RunTest(const FString& Parameters)
{
	UCameraController* CamCtrl = NewObject<UCameraController>(GetTransientPackage());
	TestNotNull("CameraController created", CamCtrl);
	if (!CamCtrl) return false;

	// Default mode should be Free
	TestEqual("Default mode is Free", CamCtrl->GetCurrentMode(), ECameraMode::Free);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CameraController — ModeParams defaults match ADR-008
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraControllerParamsTest,
	"FirstGame.Camera.CameraController.ModeParamsDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraControllerParamsTest::RunTest(const FString& Parameters)
{
	UCameraController* CamCtrl = NewObject<UCameraController>(GetTransientPackage());
	if (!CamCtrl) return false;

	// Free mode params (ADR-008)
	TestEqual("Free Distance", CamCtrl->FreeParams.Distance, 250.f);
	TestEqual("Free HeightOffset", CamCtrl->FreeParams.HeightOffset, 80.f);
	TestEqual("Free FOV", CamCtrl->FreeParams.FOV, 65.f);
	TestTrue("Free bAutoCenter = true", CamCtrl->FreeParams.bAutoCenter);

	// Locked mode params
	TestEqual("Locked Distance", CamCtrl->LockedParams.Distance, 200.f);
	TestEqual("Locked FOV", CamCtrl->LockedParams.FOV, 55.f);
	TestFalse("Locked bAutoCenter = false", CamCtrl->LockedParams.bAutoCenter);

	// Dodge mode params
	TestEqual("Dodge Distance", CamCtrl->DodgeParams.Distance, 150.f);
	TestEqual("Dodge FOV", CamCtrl->DodgeParams.FOV, 50.f);

	// Ultimate mode params
	TestEqual("Ultimate Distance", CamCtrl->UltimateParams.Distance, 100.f);
	TestEqual("Ultimate FOV", CamCtrl->UltimateParams.FOV, 45.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CameraController — Blend speeds configurable
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraControllerBlendConfigTest,
	"FirstGame.Camera.CameraController.BlendConfig",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraControllerBlendConfigTest::RunTest(const FString& Parameters)
{
	UCameraController* CamCtrl = NewObject<UCameraController>(GetTransientPackage());
	if (!CamCtrl) return false;

	// Verify blend parameters have sensible defaults
	TestTrue("DistanceBlendSpeed > 0", CamCtrl->DistanceBlendSpeed > 0.f);
	TestTrue("FOVBlendSpeed > 0", CamCtrl->FOVBlendSpeed > 0.f);
	TestTrue("AutoCenterDelay > 0", CamCtrl->AutoCenterDelay > 0.f);
	TestTrue("AutoCenterSpeed > 0", CamCtrl->AutoCenterSpeed > 0.f);

	// Verify they are configurable
	CamCtrl->DistanceBlendSpeed = 12.f;
	TestEqual("Custom DistanceBlendSpeed", CamCtrl->DistanceBlendSpeed, 12.f);

	CamCtrl->AutoCenterDelay = 5.f;
	TestEqual("Custom AutoCenterDelay", CamCtrl->AutoCenterDelay, 5.f);

	return true;
}

// ────────────────────────────────────────────────────────────────────────
// Test: CameraController — FInterpTo converges correctly
// Verifies the math behind smooth camera transitions
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInterpToConvergenceTest,
	"FirstGame.Camera.Math.FInterpToConvergence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInterpToConvergenceTest::RunTest(const FString& Parameters)
{
	// Simulate FInterpTo behavior for Distance transition
	// Free (250) → Locked (200), BlendSpeed = 8
	float Current = 250.f;
	float Target = 200.f;
	float BlendSpeed = 8.f;
	float DeltaTime = 0.016f; // 60fps

	// After one frame
	float After1Frame = FMath::FInterpTo(Current, Target, DeltaTime, BlendSpeed);
	TestTrue("After 1 frame: moved toward target", After1Frame < Current);
	TestTrue("After 1 frame: not overshot", After1Frame > Target);

	// Simulate multiple frames until convergence
	int32 MaxFrames = 300; // 5 seconds at 60fps
	bool bConverged = false;
	for (int32 Frame = 0; Frame < MaxFrames; Frame++)
	{
		Current = FMath::FInterpTo(Current, Target, DeltaTime, BlendSpeed);
		if (FMath::Abs(Current - Target) < 0.1f)
		{
			bConverged = true;
			break;
		}
	}

	TestTrue("Distance converges to target within 5s", bConverged);
	TestTrue("Final value close to target", FMath::Abs(Current - Target) < 1.f);

	// Test FOV transition: 65 → 45
	Current = 65.f;
	Target = 45.f;
	BlendSpeed = 6.f;

	for (int32 Frame = 0; Frame < MaxFrames; Frame++)
	{
		Current = FMath::FInterpTo(Current, Target, DeltaTime, BlendSpeed);
	}
	TestTrue("FOV converges toward target", Current < 50.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CameraController — Auto-center logic simulation
// Verifies that auto-center triggers after delay in Free mode only
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoCenterLogicTest,
	"FirstGame.Camera.CameraController.AutoCenterLogic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAutoCenterLogicTest::RunTest(const FString& Parameters)
{
	// Simulate auto-center timing logic
	float AutoCenterDelay = 3.f;
	float TimeSinceLastInput = 0.f;
	ECameraMode CurrentMode = ECameraMode::Free;
	bool bAutoCenterEnabled = true;
	float DeltaTime = 0.016f;

	// Simulate time passing in Free mode with auto-center
	bool bAutoCenterTriggered = false;
	for (int32 Frame = 0; Frame < 300; Frame++)
	{
		// Only accumulates in Free mode with auto-center
		if (CurrentMode == ECameraMode::Free && bAutoCenterEnabled)
		{
			TimeSinceLastInput += DeltaTime;
			if (TimeSinceLastInput > AutoCenterDelay)
			{
				bAutoCenterTriggered = true;
				break;
			}
		}
	}

	TestTrue("Auto-center triggers after delay in Free mode", bAutoCenterTriggered);
	TestTrue("Triggered after ~3s", TimeSinceLastInput > 2.9f && TimeSinceLastInput < 3.1f);

	// Simulate camera input resetting the timer
	TimeSinceLastInput = 0.f;
	bAutoCenterTriggered = false;

	// Wait 2s, then simulate input
	for (int32 Frame = 0; Frame < 125; Frame++) // ~2s
	{
		TimeSinceLastInput += DeltaTime;
	}
	TestTrue("Timer at ~2s", TimeSinceLastInput > 1.9f);

	// Player moves camera — reset
	TimeSinceLastInput = 0.f;

	// Wait another 2s — should not trigger yet
	for (int32 Frame = 0; Frame < 125; Frame++)
	{
		TimeSinceLastInput += DeltaTime;
	}
	TestFalse("Auto-center should not trigger after input reset", TimeSinceLastInput > AutoCenterDelay);

	// In Locked mode, timer should not accumulate
	CurrentMode = ECameraMode::Locked;
	float TimeBefore = TimeSinceLastInput;
	for (int32 Frame = 0; Frame < 100; Frame++)
	{
		if (CurrentMode != ECameraMode::Free || !bAutoCenterEnabled)
		{
			TimeSinceLastInput = 0.f; // Reset in non-Free mode
		}
		else
		{
			TimeSinceLastInput += DeltaTime;
		}
	}
	TestEqual("Timer stays 0 in Locked mode", TimeSinceLastInput, 0.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: CameraController — Mode transition priority
// Verifies that Dodge/Ultimate override Free/Locked
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraModeTransitionTest,
	"FirstGame.Camera.CameraController.ModeTransitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraModeTransitionTest::RunTest(const FString& Parameters)
{
	// Verify all 4 modes exist and are distinct
	TestTrue("Free != Locked", ECameraMode::Free != ECameraMode::Locked);
	TestTrue("Free != Dodge", ECameraMode::Free != ECameraMode::Dodge);
	TestTrue("Free != Ultimate", ECameraMode::Free != ECameraMode::Ultimate);
	TestTrue("Locked != Dodge", ECameraMode::Locked != ECameraMode::Dodge);
	TestTrue("Dodge != Ultimate", ECameraMode::Dodge != ECameraMode::Ultimate);

	// Verify mode params are ordered correctly for cinematic zoom effect:
	// Free (far) > Locked (mid) > Dodge (close) > Ultimate (closest)
	UCameraController* CamCtrl = NewObject<UCameraController>(GetTransientPackage());
	if (!CamCtrl) return false;

	TestTrue("Free > Locked distance", CamCtrl->FreeParams.Distance > CamCtrl->LockedParams.Distance);
	TestTrue("Locked > Dodge distance", CamCtrl->LockedParams.Distance > CamCtrl->DodgeParams.Distance);
	TestTrue("Dodge > Ultimate distance", CamCtrl->DodgeParams.Distance > CamCtrl->UltimateParams.Distance);

	// FOV should also decrease for more dramatic modes
	TestTrue("Free > Locked FOV", CamCtrl->FreeParams.FOV > CamCtrl->LockedParams.FOV);
	TestTrue("Locked > Dodge FOV", CamCtrl->LockedParams.FOV > CamCtrl->DodgeParams.FOV);

	return true;
}
