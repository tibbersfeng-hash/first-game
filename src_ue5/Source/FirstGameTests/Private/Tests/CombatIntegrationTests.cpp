// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Combat Integration Tests — verifies cross-system combat flow
// Covers: ComboManager Tick, HitStopManager Tick, PlayerCharacter damage/energy,
//         BaseEnemy death flow, full combat loop simulation

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "Combat/ComboManager.h"
#include "Combat/HitStopManager.h"
#include "Characters/PlayerCharacter.h"
#include "Characters/BaseEnemy.h"
#include "DataAssets/CharacterDataAsset.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager Tick — combo drops after window timeout
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerTickDropTest,
	"FirstGame.Combat.ComboManager.TickComboDrop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FComboManagerTickDropTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	TestNotNull("ComboManager created", Combo);
	if (!Combo) return false;

	// Register a hit to start combo
	Combo->RegisterHit("LightAttack");
	TestEqual("Combo count = 1", Combo->GetCurrentCount(), 1);
	TestTrue("IsInCombo = true", Combo->IsInCombo());

	// Simulate time passing within the combo window (0.5s < 1.5s window)
	Combo->TickComponent(0.5f, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	TestTrue("Still in combo after 0.5s", Combo->IsInCombo());
	TestEqual("Count still 1", Combo->GetCurrentCount(), 1);

	// Simulate more time — total 2.0s > 1.5s window → should drop
	Combo->TickComponent(1.5f, ELevelTick::LEVELTICK_TimeOnly, nullptr);

	TestFalse("Combo dropped after timeout", Combo->IsInCombo());
	TestEqual("Count = 0 after drop", Combo->GetCurrentCount(), 0);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager Tick — hit within window resets timer
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerTickResetTimerTest,
	"FirstGame.Combat.ComboManager.TickHitResetsTimer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FComboManagerTickResetTimerTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	if (!Combo) return false;

	// Hit 1
	Combo->RegisterHit("LightAttack");

	// Wait 1.0s (within 1.5s window)
	Combo->TickComponent(1.0f, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	TestTrue("Still in combo at 1.0s", Combo->IsInCombo());

	// Hit 2 — should reset the timer
	Combo->RegisterHit("LightAttack");
	TestEqual("Count = 2 after second hit", Combo->GetCurrentCount(), 2);

	// Wait 1.0s more — total 2.0s since first hit, but only 1.0s since second
	Combo->TickComponent(1.0f, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	TestTrue("Still in combo — timer was reset by second hit", Combo->IsInCombo());

	// Wait 0.6s more — total 1.6s since second hit → should drop
	Combo->TickComponent(0.6f, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	TestFalse("Combo dropped after 1.6s since last hit", Combo->IsInCombo());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: ComboManager Tick — custom window time
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComboManagerCustomWindowTest,
	"FirstGame.Combat.ComboManager.CustomWindowTime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FComboManagerCustomWindowTest::RunTest(const FString& Parameters)
{
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	if (!Combo) return false;

	// Set a short combo window
	Combo->ComboWindowTime = 0.5f;

	Combo->RegisterHit("LightAttack");
	TestTrue("In combo", Combo->IsInCombo());

	// Wait 0.3s (within 0.5s window)
	Combo->TickComponent(0.3f, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	TestTrue("Still in combo at 0.3s", Combo->IsInCombo());

	// Wait 0.3s more — total 0.6s > 0.5s window
	Combo->TickComponent(0.3f, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	TestFalse("Combo dropped with custom 0.5s window", Combo->IsInCombo());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: HitStopManager Tick — countdown and state transition
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHitStopManagerTickTest,
	"FirstGame.Combat.HitStopManager.TickCountdown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHitStopManagerTickTest::RunTest(const FString& Parameters)
{
	UHitStopManager* HitStop = NewObject<UHitStopManager>(GetTransientPackage());
	TestNotNull("HitStopManager created", HitStop);
	if (!HitStop) return false;

	// Initially not in hit stop
	TestFalse("Initially not in hit stop", HitStop->IsInHitStop());
	TestEqual("Remaining time = 0", HitStop->GetRemainingTime(), 0.f);

	// Request hit stop for 10 frames (~0.167s at 60fps)
	float Duration = 10.f / 60.f;
	HitStop->RequestHitStop(Duration);

	TestTrue("In hit stop after request", HitStop->IsInHitStop());
	TestTrue("Remaining time > 0", HitStop->GetRemainingTime() > 0.f);

	// Tick 5 frames worth (0.083s)
	float Tick5Frames = 5.f / 60.f;
	HitStop->TickComponent(Tick5Frames, ELevelTick::LEVELTICK_TimeOnly, nullptr);

	TestTrue("Still in hit stop after 5 frames", HitStop->IsInHitStop());
	TestTrue("Remaining time decreased", HitStop->GetRemainingTime() < Duration);

	// Tick remaining time — should exit hit stop
	float Remaining = HitStop->GetRemainingTime();
	HitStop->TickComponent(Remaining + 0.01f, ELevelTick::LEVELTICK_TimeOnly, nullptr);

	TestFalse("Exited hit stop after countdown", HitStop->IsInHitStop());
	TestEqual("Remaining time = 0", HitStop->GetRemainingTime(), 0.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: HitStopManager — longer request overrides shorter
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHitStopManagerOverrideTest,
	"FirstGame.Combat.HitStopManager.LongerOverrides",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHitStopManagerOverrideTest::RunTest(const FString& Parameters)
{
	UHitStopManager* HitStop = NewObject<UHitStopManager>(GetTransientPackage());
	if (!HitStop) return false;

	// Request short hit stop (3 frames)
	HitStop->RequestHitStop(3.f / 60.f);
	float ShortRemaining = HitStop->GetRemainingTime();

	// Request longer hit stop (10 frames) — should override
	HitStop->RequestHitStop(10.f / 60.f);
	float LongRemaining = HitStop->GetRemainingTime();

	TestTrue("Longer request overrode shorter", LongRemaining > ShortRemaining);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: PlayerCharacter — Initialize, health, CanAct
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerCharacterInitTest,
	"FirstGame.Combat.PlayerCharacter.Initialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlayerCharacterInitTest::RunTest(const FString& Parameters)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(GetTransientPackage());
	Data->CharacterId = FName("Huikong");
	Data->MaxHealth = 100.f;
	Data->MaxEnergy = 100.f;
	Data->MoveSpeed = 320.f;

	APlayerCharacter* Player = NewObject<APlayerCharacter>(GetTransientPackage());
	TestNotNull("PlayerCharacter created", Player);
	if (!Player) return false;

	Player->InitializeCharacter(Data);

	TestEqual("Health initialized to max", Player->CurrentHealth, 100.f);
	TestEqual("Energy initialized to max", Player->CurrentEnergy, 100.f);
	TestEqual("CharacterId set", Player->CurrentCharacterId, FName("Huikong"));
	TestTrue("CanAct in default state", Player->CanAct());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: PlayerCharacter — ReceiveHitDamage and Heal flow
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerCharacterDamageTest,
	"FirstGame.Combat.PlayerCharacter.ReceiveHitDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlayerCharacterDamageTest::RunTest(const FString& Parameters)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(GetTransientPackage());
	Data->MaxHealth = 100.f;

	APlayerCharacter* Player = NewObject<APlayerCharacter>(GetTransientPackage());
	if (!Player) return false;
	Player->InitializeCharacter(Data);

	// Take damage
	Player->ReceiveHitDamage(25.f, nullptr);
	TestEqual("Health after 25 damage", Player->CurrentHealth, 75.f);

	// Take more damage
	Player->ReceiveHitDamage(50.f, nullptr);
	TestEqual("Health after 75 total damage", Player->CurrentHealth, 25.f);

	// Heal
	Player->Heal(30.f);
	TestEqual("Health after heal", Player->CurrentHealth, 55.f);

	// Over-heal should clamp to max
	Player->Heal(200.f);
	TestEqual("Health clamped to max", Player->CurrentHealth, 100.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: PlayerCharacter — energy system
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerCharacterEnergyTest,
	"FirstGame.Combat.PlayerCharacter.EnergySystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlayerCharacterEnergyTest::RunTest(const FString& Parameters)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(GetTransientPackage());
	Data->MaxEnergy = 100.f;
	Data->EnergyCostPerAttack = 20.f;

	APlayerCharacter* Player = NewObject<APlayerCharacter>(GetTransientPackage());
	if (!Player) return false;
	Player->InitializeCharacter(Data);

	// Initial energy
	TestEqual("Initial energy = max", Player->CurrentEnergy, 100.f);

	// Has enough energy
	TestTrue("Has enough for 20 energy attack", Player->HasEnoughEnergy(20.f));
	TestFalse("Not enough for 200 energy attack", Player->HasEnoughEnergy(200.f));

	// Consume energy
	bool bConsumed = Player->ConsumeEnergy(20.f);
	TestTrue("ConsumeEnergy succeeded", bConsumed);
	TestEqual("Energy after attack", Player->CurrentEnergy, 80.f);

	// Add energy (pickup)
	Player->AddEnergy(15.f);
	TestEqual("Energy after pickup", Player->CurrentEnergy, 95.f);

	// Over-add should clamp
	Player->AddEnergy(100.f);
	TestEqual("Energy clamped to max", Player->CurrentEnergy, 100.f);

	// Consume more than available — should fail or clamp to 0
	Player->CurrentEnergy = 10.f;
	bConsumed = Player->ConsumeEnergy(20.f);
	// Either returns false or clamps to 0
	TestTrue("Energy >= 0 after over-drain", Player->CurrentEnergy >= 0.f);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: BaseEnemy — damage, death flow
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBaseEnemyDeathTest,
	"FirstGame.Combat.BaseEnemy.DeathFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBaseEnemyDeathTest::RunTest(const FString& Parameters)
{
	UCharacterDataAsset* Data = NewObject<UCharacterDataAsset>(GetTransientPackage());
	Data->MaxHealth = 50.f;

	ABaseEnemy* Enemy = NewObject<ABaseEnemy>(GetTransientPackage());
	TestNotNull("BaseEnemy created", Enemy);
	if (!Enemy) return false;

	Enemy->InitializeEnemy(Data);
	TestEqual("Enemy health = max", Enemy->CurrentHealth, 50.f);
	TestEqual("Default state = Idle", Enemy->GetCurrentState(), FName("Idle"));

	// Take damage
	Enemy->ReceiveHitDamage(20.f, nullptr);
	TestEqual("Health after 20 damage", Enemy->CurrentHealth, 30.f);

	// Take more damage
	Enemy->ReceiveHitDamage(25.f, nullptr);
	TestEqual("Health after 45 damage", Enemy->CurrentHealth, 5.f);

	// Lethal damage
	Enemy->ReceiveHitDamage(100.f, nullptr);
	TestEqual("Health clamped to 0", Enemy->CurrentHealth, 0.f);

	// State should have changed from Idle
	FName StateAfterDeath = Enemy->GetCurrentState();
	TestTrue("State changed from Idle", StateAfterDeath != FName("Idle"));

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: BaseEnemy — aggro and lock-on properties
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBaseEnemyAggroLockTest,
	"FirstGame.Combat.BaseEnemy.AggroAndLock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBaseEnemyAggroLockTest::RunTest(const FString& Parameters)
{
	ABaseEnemy* Enemy = NewObject<ABaseEnemy>(GetTransientPackage());
	if (!Enemy) return false;

	// Default state
	TestFalse("Default: not aggro", Enemy->IsAggro());

	// Set aggro
	Enemy->SetAggro(true);
	TestTrue("After SetAggro(true): aggro", Enemy->IsAggro());

	Enemy->SetAggro(false);
	TestFalse("After SetAggro(false): not aggro", Enemy->IsAggro());

	// Lock-on: full HP enemy should be lockable with ratio 1.0
	float HPRatio = Enemy->GetHPRatio_Implementation();
	TestEqual("Full HP ratio = 1.0", HPRatio, 1.0f);

	bool bLockable = Enemy->IsLockable_Implementation();
	TestTrue("Full HP enemy is lockable", bLockable);

	// Damage and recheck
	Enemy->CurrentHealth = 25.f;
	HPRatio = Enemy->GetHPRatio_Implementation();
	TestTrue("Damaged HP ratio < 1.0", HPRatio < 1.0);
	TestTrue("Damaged enemy still lockable", Enemy->IsLockable_Implementation());

	// Zero HP — should not be lockable
	Enemy->CurrentHealth = 0.f;
	TestFalse("Dead enemy not lockable", Enemy->IsLockable_Implementation());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: Full Combat Loop — Player attacks → Enemy dies
// Simulates the entire combat flow without World context
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatFullLoopTest,
	"FirstGame.Integration.FullCombatLoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCombatFullLoopTest::RunTest(const FString& Parameters)
{
	// ── Setup ──────────────────────────────────────────────────────
	UCharacterDataAsset* PlayerData = NewObject<UCharacterDataAsset>(GetTransientPackage());
	PlayerData->MaxHealth = 100.f;
	PlayerData->MaxEnergy = 100.f;
	PlayerData->EnergyCostPerAttack = 10.f;

	UCharacterDataAsset* EnemyData = NewObject<UCharacterDataAsset>(GetTransientPackage());
	EnemyData->MaxHealth = 30.f;

	APlayerCharacter* Player = NewObject<APlayerCharacter>(GetTransientPackage());
	ABaseEnemy* Enemy = NewObject<ABaseEnemy>(GetTransientPackage());
	UComboManager* Combo = NewObject<UComboManager>(GetTransientPackage());
	UHitStopManager* HitStop = NewObject<UHitStopManager>(GetTransientPackage());

	if (!Player || !Enemy || !Combo || !HitStop) return false;

	Player->InitializeCharacter(PlayerData);
	Enemy->InitializeEnemy(EnemyData);

	// ── Combat Round 1: Light Attack ──────────────────────────────
	float LightDamage = 10.f;

	// Player consumes energy
	bool bCanAttack = Player->HasEnoughEnergy(PlayerData->EnergyCostPerAttack);
	TestTrue("Player can afford light attack", bCanAttack);

	Player->ConsumeEnergy(PlayerData->EnergyCostPerAttack);
	TestTrue("Energy >= 0 after consume", Player->CurrentEnergy >= 0.f);

	// Combo registers
	Combo->RegisterHit("LightAttack");
	TestEqual("Combo = 1", Combo->GetCurrentCount(), 1);

	// HitStop triggered (5 frames)
	HitStop->RequestHitStop(5.f / 60.f);
	TestTrue("HitStop active", HitStop->IsInHitStop());

	// Enemy takes damage
	Enemy->ReceiveHitDamage(LightDamage, Player);
	TestEqual("Enemy HP: 30→20", Enemy->CurrentHealth, 20.f);

	// ─ Combat Round 2: Light Attack ──────────────────────────────
	Player->ConsumeEnergy(PlayerData->EnergyCostPerAttack);
	Combo->RegisterHit("LightAttack");
	TestEqual("Combo = 2", Combo->GetCurrentCount(), 2);

	Enemy->ReceiveHitDamage(LightDamage, Player);
	TestEqual("Enemy HP: 20→10", Enemy->CurrentHealth, 10.f);

	// ── Combat Round 3: Heavy Attack (finishing blow) ──────────────
	float HeavyDamage = 15.f;
	Player->ConsumeEnergy(PlayerData->EnergyCostPerAttack * 2);
	Combo->RegisterHit("HeavyAttack");
	TestEqual("Combo = 3", Combo->GetCurrentCount(), 3);

	Enemy->ReceiveHitDamage(HeavyDamage, Player);
	TestEqual("Enemy HP clamped to 0", Enemy->CurrentHealth, 0.f);

	// Enemy state should not be Idle anymore
	TestTrue("Enemy not in Idle state", Enemy->GetCurrentState() != FName("Idle"));

	// ── Post-combat: combo drops after timeout ─────────────────────
	Combo->TickComponent(2.0f, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	TestFalse("Combo dropped after combat", Combo->IsInCombo());

	// ── HitStop expires ────────────────────────────────────────────
	float HitStopRemaining = HitStop->GetRemainingTime();
	HitStop->TickComponent(HitStopRemaining + 0.01f, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	TestFalse("HitStop expired", HitStop->IsInHitStop());

	// ── Player survived ────────────────────────────────────────────
	TestTrue("Player still alive", Player->CurrentHealth > 0.f);
	TestTrue("Player still has energy", Player->CurrentEnergy >= 0.f);

	return true;
}
