// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// New Module Tests — Achievements, Equipment, Skills, Dialogue, Social

#include "FirstGameTests.h"
#include "Misc/AutomationTest.h"
#include "DataAssets/EquipmentAsset.h"
#include "DataAssets/SkillNodeData.h"
#include "Achievements/AchievementData.h"
#include "Dialogue/DialogueData.h"
#include "Social/FriendManager.h"

// ─────────────────────────────────────────────────────────────────────────
// Test: EquipmentAsset — Rarity and type enums
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(EEquipmentEnumTest,
	"FirstGame.Modules.Equipment.Enums",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool EEquipmentEnumTest::RunTest(const FString& Parameters)
{
	// Verify equipment types are distinct
	TestTrue("Weapon != Armor", EEquipmentType::Weapon != EEquipmentType::Armor);
	TestTrue("Armor != Accessory", EEquipmentType::Armor != EEquipmentType::Accessory);
	TestTrue("Accessory != Boots", EEquipmentType::Accessory != EEquipmentType::Boots);

	// Verify rarity ordering (Common < Uncommon < Rare < Epic < Legendary)
	TestTrue("Common < Uncommon", (int)EEquipmentRarity::Common < (int)EEquipmentRarity::Uncommon);
	TestTrue("Uncommon < Rare", (int)EEquipmentRarity::Uncommon < (int)EEquipmentRarity::Rare);
	TestTrue("Rare < Epic", (int)EEquipmentRarity::Rare < (int)EEquipmentRarity::Epic);
	TestTrue("Epic < Legendary", (int)EEquipmentRarity::Epic < (int)EEquipmentRarity::Legendary);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: EquipmentStatBonus — Stat accumulation
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEquipmentStatBonusTest,
	"FirstGame.Modules.Equipment.StatBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEquipmentStatBonusTest::RunTest(const FString& Parameters)
{
	FEquipmentStatBonus Bonus1;
	Bonus1.MaxHealth = 50.f;
	Bonus1.AttackDamage = 10.f;
	Bonus1.Defense = 5.f;

	FEquipmentStatBonus Bonus2;
	Bonus2.MaxHealth = 30.f;
	Bonus2.AttackDamage = 15.f;
	Bonus2.MoveSpeed = 10.f;

	// Simulate stacking two equipment bonuses
	FEquipmentStatBonus Total;
	Total.MaxHealth = Bonus1.MaxHealth + Bonus2.MaxHealth;
	Total.AttackDamage = Bonus1.AttackDamage + Bonus2.AttackDamage;
	Total.Defense = Bonus1.Defense + Bonus2.Defense;
	Total.MoveSpeed = Bonus1.MoveSpeed + Bonus2.MoveSpeed;

	TestEqual("Total MaxHealth", Total.MaxHealth, 80.f);
	TestEqual("Total AttackDamage", Total.AttackDamage, 25.f);
	TestEqual("Total Defense", Total.Defense, 5.f);
	TestEqual("Total MoveSpeed", Total.MoveSpeed, 10.f);

	return true;
}

// ────────────────────────────────────────────────────────────────────────
// Test: SkillNodeData — Unlock and upgrade logic
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSkillNodeUnlockTest,
	"FirstGame.Modules.Skills.SkillNodeUnlock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSkillNodeUnlockTest::RunTest(const FString& Parameters)
{
	USkillNodeData* Skill = NewObject<USkillNodeData>(GetTransientPackage());
	TestNotNull("SkillNodeData created", Skill);
	if (!Skill) return false;

	Skill->RequiredPoints = 3;
	Skill->RequiredCharacterLevel = 5;
	Skill->MaxLevel = 5;

	// Can't unlock with insufficient points
	bool bCanUnlock1 = Skill->CanUnlock(2, 10);
	TestFalse("Can't unlock with 2 points (need 3)", bCanUnlock1);

	// Can't unlock with insufficient level
	bool bCanUnlock2 = Skill->CanUnlock(5, 3);
	TestFalse("Can't unlock at level 3 (need 5)", bCanUnlock2);

	// Can unlock with enough points and level
	bool bCanUnlock3 = Skill->CanUnlock(5, 10);
	TestTrue("Can unlock with 5 points and level 10", bCanUnlock3);

	// Initial state
	TestFalse("Initially not unlocked", Skill->bIsUnlocked);
	TestFalse("Initially not upgraded", Skill->bIsUpgraded);

	// Unlock
	Skill->Unlock();
	TestTrue("After Unlock: bIsUnlocked", Skill->bIsUnlocked);

	// Upgrade to max level (MaxLevel=5, SkillLevel starts at 1, need 4 upgrades)
	for (int32 i = 0; i < 4; i++)
	{
		bool bUpgraded = Skill->Upgrade();
		TestTrue("Upgrade succeeded", bUpgraded);
	}
	TestEqual("SkillLevel at max", Skill->SkillLevel, 5);
	TestTrue("After max upgrade: bIsUpgraded", Skill->bIsUpgraded);

	// Can't upgrade past max
	bool bUpgradePastMax = Skill->Upgrade();
	TestFalse("Can't upgrade past max level", bUpgradePastMax);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: SkillNodeData — Prerequisites
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSkillNodePrerequisitesTest,
	"FirstGame.Modules.Skills.Prerequisites",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSkillNodePrerequisitesTest::RunTest(const FString& Parameters)
{
	USkillNodeData* Skill = NewObject<USkillNodeData>(GetTransientPackage());
	if (!Skill) return false;

	// No prerequisites
	TestEqual("Initial prerequisites = 0", Skill->PrerequisiteSkills.Num(), 0);

	// Add prerequisites
	Skill->PrerequisiteSkills.Add(FName("Slash"));
	Skill->PrerequisiteSkills.Add(FName("Thrust"));
	TestEqual("After adding 2 prerequisites", Skill->PrerequisiteSkills.Num(), 2);

	// Verify contains
	bool bHasSlash = Skill->PrerequisiteSkills.Contains(FName("Slash"));
	bool bHasKick = Skill->PrerequisiteSkills.Contains(FName("Kick"));
	TestTrue("Contains Slash", bHasSlash);
	TestFalse("Does not contain Kick", bHasKick);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: AchievementData — Progress tracking
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementProgressTest,
	"FirstGame.Modules.Achievements.Progress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAchievementProgressTest::RunTest(const FString& Parameters)
{
	UAchievementData* Achievement = NewObject<UAchievementData>(GetTransientPackage());
	TestNotNull("AchievementData created", Achievement);
	if (!Achievement) return false;

	Achievement->RequiredProgress = 10;
	Achievement->CurrentProgress = 0;

	// Initial state
	TestFalse("Initially not unlocked", Achievement->bIsUnlocked);
	TestEqual("Initial progress = 0", Achievement->CurrentProgress, 0);

	// Progress increment
	Achievement->CurrentProgress += 5;
	TestEqual("Progress = 5", Achievement->CurrentProgress, 5);
	TestFalse("Not unlocked at 5/10", Achievement->bIsUnlocked);

	// Reach threshold
	Achievement->CurrentProgress += 5;
	TestEqual("Progress = 10", Achievement->CurrentProgress, 10);

	// Check unlock condition
	bool bShouldUnlock = (Achievement->CurrentProgress >= Achievement->RequiredProgress);
	TestTrue("Should unlock at 10/10", bShouldUnlock);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: DialogueData — Node structure
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDialogueNodeStructureTest,
	"FirstGame.Modules.Dialogue.NodeStructure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDialogueNodeStructureTest::RunTest(const FString& Parameters)
{
	UDialogueNodeData* Node = NewObject<UDialogueNodeData>(GetTransientPackage());
	TestNotNull("DialogueNodeData created", Node);
	if (!Node) return false;

	// Verify default state
	TestFalse("Default: not end node", Node->bIsEnd);

	// Set dialogue text
	Node->DialogueText = FText::FromString("Hello, adventurer!");
	TestTrue("Dialogue text not empty", !Node->DialogueText.IsEmpty());

	// Add dialogue choices
	FDialogueChoice Choice1;
	Choice1.ChoiceText = FText::FromString("Fight");
	Choice1.NextDialogueID = FName("dialogue_fight");
	Node->Choices.Add(Choice1);

	FDialogueChoice Choice2;
	Choice2.ChoiceText = FText::FromString("Flee");
	Choice2.NextDialogueID = FName("dialogue_flee");
	Node->Choices.Add(Choice2);

	FDialogueChoice Choice3;
	Choice3.ChoiceText = FText::FromString("Talk");
	Choice3.RequiredCondition = "has_item_key";
	Node->Choices.Add(Choice3);

	TestEqual("3 choices", Node->Choices.Num(), 3);

	// Verify choice properties
	TestTrue("Choice 1 = Fight", Node->Choices[0].ChoiceText.EqualTo(FText::FromString("Fight")));
	TestTrue("Choice 3 has condition", !Node->Choices[2].RequiredCondition.IsEmpty());

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: FFriendData — Equality operators
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFriendDataEqualityTest,
	"FirstGame.Modules.Social.FriendDataEquality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFriendDataEqualityTest::RunTest(const FString& Parameters)
{
	FFriendData Friend1;
	Friend1.PlayerID = "player_001";
	Friend1.PlayerName = "Huikong";
	Friend1.Status = EFriendStatus::Online;

	FFriendData Friend2;
	Friend2.PlayerID = "player_001";
	Friend2.PlayerName = "Huikong";
	Friend2.Status = EFriendStatus::InGame;

	FFriendData Friend3;
	Friend3.PlayerID = "player_002";
	Friend3.PlayerName = "Stranger";

	// Equality based on PlayerID
	TestTrue("Same PlayerID = equal", Friend1 == Friend2);
	TestFalse("Different PlayerID = not equal", Friend1 == Friend3);
	TestTrue("Different PlayerID = !=", Friend1 != Friend3);

	// Status values
	TestTrue("Online != InGame", EFriendStatus::Online != EFriendStatus::InGame);
	TestTrue("Offline is default", (int)EFriendStatus::Offline == 0);

	return true;
}

// ─────────────────────────────────────────────────────────────────────────
// Test: FriendManager — Friend list operations simulation
// ─────────────────────────────────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFriendListSimulationTest,
	"FirstGame.Modules.Social.FriendListSimulation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFriendListSimulationTest::RunTest(const FString& Parameters)
{
	// Simulate friend list operations (pure logic, no Actor)
	TArray<FFriendData> FriendsList;

	// Add friends
	FFriendData Friend1;
	Friend1.PlayerID = "p001";
	Friend1.PlayerName = "Alice";
	Friend1.Status = EFriendStatus::Online;
	FriendsList.Add(Friend1);

	FFriendData Friend2;
	Friend2.PlayerID = "p002";
	Friend2.PlayerName = "Bob";
	Friend2.Status = EFriendStatus::Offline;
	FriendsList.Add(Friend2);

	FFriendData Friend3;
	Friend3.PlayerID = "p003";
	Friend3.PlayerName = "Charlie";
	Friend3.Status = EFriendStatus::InGame;
	FriendsList.Add(Friend3);

	TestEqual("3 friends", FriendsList.Num(), 3);

	// Count online friends
	int32 OnlineCount = 0;
	for (const FFriendData& Friend : FriendsList)
	{
		if (Friend.Status == EFriendStatus::Online || Friend.Status == EFriendStatus::InGame)
		{
			OnlineCount++;
		}
	}
	TestEqual("2 friends online/in-game", OnlineCount, 2);

	// Remove a friend
	FriendsList.RemoveAll([&](const FFriendData& F) { return F.PlayerID == "p002"; });
	TestEqual("2 friends after removal", FriendsList.Num(), 2);

	// Verify Bob is removed
	bool bBobExists = false;
	for (const FFriendData& Friend : FriendsList)
	{
		if (Friend.PlayerID == "p002")
		{
			bBobExists = true;
			break;
		}
	}
	TestFalse("Bob removed", bBobExists);

	return true;
}
