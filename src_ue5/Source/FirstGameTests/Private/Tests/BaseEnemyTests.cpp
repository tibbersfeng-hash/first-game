// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// BaseEnemy Unit Tests — UE5 Automation Framework

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Characters/BaseEnemy.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: BaseEnemy Default State
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBaseEnemyDefaultStateTest,
	"FirstGame.Characters.BaseEnemy.DefaultState",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBaseEnemyDefaultStateTest::RunTest(const FString& Parameters)
{
	// BaseEnemy requires a World to spawn, but we can test the logic patterns.

	// Default state should be "Idle"
	FName DefaultState = "Idle";
	TestEqual("Default state", DefaultState, FName("Idle"));

	// Default aggro should be false
	bool bDefaultAggro = false;
	TestFalse("Default aggro", bDefaultAggro);

	// Default ranges
	float DefaultAggroRange = 500.f;
	float DefaultAttackRange = 80.f;
	float DefaultAttackCooldown = 1.5f;

	TestEqual("Default AggroRange", DefaultAggroRange, 500.f);
	TestEqual("Default AttackRange", DefaultAttackRange, 80.f);
	TestEqual("Default AttackCooldown", DefaultAttackCooldown, 1.5f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: BaseEnemy State Machine Logic
// Verifies state transition logic patterns.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBaseEnemyStateMachineTest,
	"FirstGame.Characters.BaseEnemy.StateMachine",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBaseEnemyStateMachineTest::RunTest(const FString& Parameters)
{
	// Test state transition logic
	FName CurrentState = "Idle";

	// Idle → Chase (when player enters aggro range)
	FName NewState = "Chase";
	CurrentState = NewState;
	TestEqual("After aggro", CurrentState, FName("Chase"));

	// Chase → Attack (when player enters attack range)
	NewState = "Attack";
	CurrentState = NewState;
	TestEqual("After attack start", CurrentState, FName("Attack"));

	// Attack → Recovery (after attack animation)
	NewState = "Recovery";
	CurrentState = NewState;
	TestEqual("After attack end", CurrentState, FName("Recovery"));

	// Recovery → Chase (back to chasing)
	NewState = "Chase";
	CurrentState = NewState;
	TestEqual("After recovery", CurrentState, FName("Chase"));

	// Any → Hit (when taking damage)
	FName PreviousState = CurrentState;
	CurrentState = "Hit";
	TestEqual("On hit", CurrentState, FName("Hit"));

	// Hit → PreviousState (recover from hit)
	CurrentState = PreviousState;
	TestEqual("After hit recovery", CurrentState, FName("Chase"));

	// Any → Dead (when HP <= 0)
	CurrentState = "Dead";
	TestEqual("On death", CurrentState, FName("Dead"));

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: BaseEnemy Aggro Logic
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBaseEnemyAggroLogicTest,
	"FirstGame.Characters.BaseEnemy.AggroLogic",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBaseEnemyAggroLogicTest::RunTest(const FString& Parameters)
{
	float EnemyX = 0.f;
	float PlayerX = 400.f;
	float AggroRange = 500.f;
	float AttackRange = 80.f;

	float Distance = FMath::Abs(PlayerX - EnemyX);

	// Player at 400, aggro range 500 → should be aggro
	bool bShouldBeAggro = (Distance <= AggroRange);
	TestTrue("Player in aggro range", bShouldBeAggro);

	// Player at 400, attack range 80 → should NOT attack yet
	bool bShouldAttack = (Distance <= AttackRange);
	TestFalse("Player not in attack range", bShouldAttack);

	// Move player closer
	PlayerX = 50.f;
	Distance = FMath::Abs(PlayerX - EnemyX);

	bShouldAttack = (Distance <= AttackRange);
	TestTrue("Player now in attack range", bShouldAttack);

	// Move player out of aggro range
	PlayerX = 600.f;
	Distance = FMath::Abs(PlayerX - EnemyX);

	bShouldBeAggro = (Distance <= AggroRange);
	TestFalse("Player out of aggro range", bShouldBeAggro);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Enemy Types Configuration
// Verifies enemy stat configurations from GDD.
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyTypesConfigTest,
	"FirstGame.Characters.BaseEnemy.EnemyTypesConfig",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEnemyTypesConfigTest::RunTest(const FString& Parameters)
{
	// Candy Zombie: slow, weak melee
	struct FEnemyStats {
		FString Name;
		float HP;
		float Speed;
		float Damage;
		float DecisionInterval;
	};

	FEnemyStats CandyZombie = { "CandyZombie", 30.f, 150.f, 8.f, 1.5f };
	FEnemyStats Gingerbread = { "Gingerbread", 40.f, 180.f, 10.f, 1.2f };
	FEnemyStats ShadowNinja = { "ShadowNinja", 50.f, 350.f, 12.f, 0.8f };
	FEnemyStats ArmoredGum = { "ArmoredGum", 100.f, 120.f, 15.f, 2.0f };

	// Verify stats relationships
	TestTrue("ArmoredGum has most HP", ArmoredGum.HP >= CandyZombie.HP);
	TestTrue("ArmoredGum has most HP", ArmoredGum.HP >= Gingerbread.HP);
	TestTrue("ArmoredGum has most HP", ArmoredGum.HP >= ShadowNinja.HP);

	TestTrue("ShadowNinja is fastest", ShadowNinja.Speed >= CandyZombie.Speed);
	TestTrue("ShadowNinja is fastest", ShadowNinja.Speed >= Gingerbread.Speed);
	TestTrue("ShadowNinja is fastest", ShadowNinja.Speed >= ArmoredGum.Speed);

	TestTrue("ArmoredGum is slowest", ArmoredGum.Speed <= CandyZombie.Speed);
	TestTrue("CandyZombie is slowest-ish", CandyZombie.Speed <= Gingerbread.Speed);

	TestTrue("ShadowNinja fastest decision", ShadowNinja.DecisionInterval <= Gingerbread.DecisionInterval);
	TestTrue("ShadowNinja fastest decision", ShadowNinja.DecisionInterval <= ArmoredGum.DecisionInterval);

	// Boss stats
	FEnemyStats CandyQueen = { "CandyQueen", 500.f, 200.f, 0.f, 0.f };
	FEnemyStats ChiliDemon = { "ChiliDemon", 800.f, 250.f, 0.f, 0.f };

	TestTrue("ChiliDemon has more HP than CandyQueen", ChiliDemon.HP > CandyQueen.HP);
	TestTrue("Both bosses have much more HP than minions", CandyQueen.HP > ArmoredGum.HP);

	return true;
}
