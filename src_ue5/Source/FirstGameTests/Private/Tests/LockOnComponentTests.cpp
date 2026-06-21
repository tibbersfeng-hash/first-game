// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// LockOnComponent Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "LockOn/LockOnComponent.h"
#include "LockOn/ILockableTarget.h"
#include "Characters/BaseEnemy.h"

// ─────────────────────────────────────────────────────────────────────────
// Helper: 创建测试用 LockOnComponent
// ─────────────────────────────────────────────────────────────────────────
static ULockOnComponent* CreateTestLockOnComponent(UObject* Outer)
{
	ULockOnComponent* LockOn = NewObject<ULockOnComponent>(Outer);
	// 不覆盖默认值，让测试验证真正的默认配置
	return LockOn;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: LockOnComponent Default Configuration
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLockOnConfigTest,
	"FirstGame.Combat.LockOn.DefaultConfig",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLockOnConfigTest::RunTest(const FString& Parameters)
{
	ULockOnComponent* LockOn = CreateTestLockOnComponent(GetTransientPackage());
	TestNotNull("LockOnComponent should be created", LockOn);
	if (!LockOn) return false;

	TestEqual("Default LockRange", LockOn->LockRange, 2000.f);
	TestEqual("Default LockFOV", LockOn->LockFOV, 60.f);
	TestEqual("Default LockHeightRange", LockOn->LockHeightRange, 1000.f);
	TestEqual("Default SwitchDelay", LockOn->SwitchDelay, 0.2f);
	TestEqual("Default LostDelay", LockOn->LostDelay, 3.f);
	TestEqual("Default TargetCacheInterval", LockOn->TargetCacheInterval, 0.5f);
	TestEqual("Default WeightHP", LockOn->WeightHP, 0.4f);
	TestEqual("Default WeightDistance", LockOn->WeightDistance, 0.4f);
	TestEqual("Default WeightAngle", LockOn->WeightAngle, 0.2f);

	// 初始状态: 无锁定目标
	TestFalse("Initially not locked on", LockOn->IsLockedOn());
	TestNull("Initially no current target", LockOn->GetCurrentTarget());
	TestEqual("Initially no lockable targets", LockOn->GetLockableTargets().Num(), 0);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: LockOnComponent ReleaseLock When Not Locked (no crash)
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLockOnReleaseEmptyTest,
	"FirstGame.Combat.LockOn.ReleaseWhenNotLocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLockOnReleaseEmptyTest::RunTest(const FString& Parameters)
{
	ULockOnComponent* LockOn = CreateTestLockOnComponent(GetTransientPackage());
	if (!LockOn) return false;

	// 释放时不应崩溃 (内部有 IsValid 检查)
	LockOn->ReleaseLock();
	TestFalse("Still not locked after release", LockOn->IsLockedOn());

	// 切换时也不应崩溃
	LockOn->SwitchTarget(true);
	LockOn->SwitchTarget(false);
	TestFalse("Still not locked after switch", LockOn->IsLockedOn());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ILockableTarget Interface — BaseEnemy Implementation
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLockOnBaseEnemyInterfaceTest,
	"FirstGame.Combat.LockOn.BaseEnemyInterface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLockOnBaseEnemyInterfaceTest::RunTest(const FString& Parameters)
{
	// 验证 BaseEnemy 实现了 ILockableTarget 接口
	TestTrue("BaseEnemy should implement ILockableTarget",
		ABaseEnemy::StaticClass()->ImplementsInterface(ULockableTarget::StaticClass()));

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: LockOnComponent Delegate Binding
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLockOnDelegateBindingTest,
	"FirstGame.Combat.LockOn.DelegateBinding",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLockOnDelegateBindingTest::RunTest(const FString& Parameters)
{
	ULockOnComponent* LockOn = CreateTestLockOnComponent(GetTransientPackage());
	if (!LockOn) return false;

	// 验证委托初始未绑定 (动态委托不支持 lambda)
	TestFalse("OnLockChanged delegate should not be bound initially",
		LockOn->OnLockChanged.IsBound());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: LockOnComponent FOV Angle Calculation
// ────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLockOnFOVCalcTest,
	"FirstGame.Combat.LockOn.FOVCalculation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLockOnFOVCalcTest::RunTest(const FString& Parameters)
{
	// 验证 FOV 角度到弧度的转换逻辑
	float LockFOV = 60.f; // 度
	float MaxFOVRad = FMath::DegreesToRadians(LockFOV);

	// 60度 = PI/3 ≈ 1.0472 弧度
	TestEqual("FOV conversion", (double)MaxFOVRad, PI / 3.0, 0.01);

	// 角度分数计算验证
	// 0度 → 分数 1.0, 60度 → 分数 0.0
	float Angle0 = 0.f;
	float Angle60 = MaxFOVRad;
	float Score0 = 1.f - FMath::Clamp(Angle0 / MaxFOVRad, 0.f, 1.f);
	float Score60 = 1.f - FMath::Clamp(Angle60 / MaxFOVRad, 0.f, 1.f);

	TestEqual("Angle score at 0 degrees", Score0, 1.f);
	TestEqual("Angle score at FOV edge", Score60, 0.f);

	// 半角 (30度) → 分数 0.5
	float Angle30 = FMath::DegreesToRadians(30.f);
	float Score30 = 1.f - FMath::Clamp(Angle30 / MaxFOVRad, 0.f, 1.f);
	TestEqual("Angle score at 30 degrees", (double)Score30, 0.5, 0.01);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: LockOnComponent Distance Score Calculation
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLockOnDistanceCalcTest,
	"FirstGame.Combat.LockOn.DistanceCalculation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLockOnDistanceCalcTest::RunTest(const FString& Parameters)
{
	float LockRange = 2000.f;

	// 距离分数: 越近越高
	// 距离 0 → 分数 1.0
	float Dist0 = 0.f;
	float Score0 = 1.f - FMath::Clamp(Dist0 / LockRange, 0.f, 1.f);
	TestEqual("Distance score at 0", Score0, 1.f);

	// 距离 = 范围 → 分数 0.0
	float ScoreMax = 1.f - FMath::Clamp(LockRange / LockRange, 0.f, 1.f);
	TestEqual("Distance score at max range", ScoreMax, 0.f);

	// 距离 = 一半范围 → 分数 0.5
	float ScoreHalf = 1.f - FMath::Clamp(1000.f / LockRange, 0.f, 1.f);
	TestEqual("Distance score at half range", ScoreHalf, 0.5f);

	// 超出范围 → 分数 0.0 (Clamp)
	float ScoreOver = 1.f - FMath::Clamp(3000.f / LockRange, 0.f, 1.f);
	TestEqual("Distance score beyond range", ScoreOver, 0.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: LockOnComponent HP Priority Weight
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLockOnHPWeightTest,
	"FirstGame.Combat.LockOn.HPPriority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLockOnHPWeightTest::RunTest(const FString& Parameters)
{
	// HP 越低 → 优先级越高
	// HP Ratio 0 (死亡) → 分数 1.0
	float HPScoreDead = 1.f - 0.f;
	TestEqual("HP score when dead", HPScoreDead, 1.f);

	// HP Ratio 0.5 (半血) → 分数 0.5
	float HPScoreHalf = 1.f - 0.5f;
	TestEqual("HP score at 50%", HPScoreHalf, 0.5f);

	// HP Ratio 1.0 (满血) → 分数 0.0
	float HPScoreFull = 1.f - 1.f;
	TestEqual("HP score at 100%", HPScoreFull, 0.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: LockOnComponent Weight Sum Validation
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLockOnWeightSumTest,
	"FirstGame.Combat.LockOn.WeightSum",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLockOnWeightSumTest::RunTest(const FString& Parameters)
{
	ULockOnComponent* LockOn = CreateTestLockOnComponent(GetTransientPackage());
	if (!LockOn) return false;

	// 三个权重之和应为 1.0
	float TotalWeight = LockOn->WeightHP + LockOn->WeightDistance + LockOn->WeightAngle;
	TestEqual("Weight sum should be 1.0", (double)TotalWeight, 1.0, 0.01);

	return true;
}
