// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "DataAssets/SkillNodeData.h"
#include "FirstGame.h"

USkillNodeData::USkillNodeData()
{
	SkillID = NAME_None;
	SkillName = FText::FromString(TEXT("未命名技能"));
	SkillType = ESkillType::Passive;
	SkillLevel = 1;
	MaxLevel = 5;
	RequiredPoints = 1;
	RequiredCharacterLevel = 1;
	bIsUnlocked = false;
	bIsUpgraded = false;
}

void USkillNodeData::Unlock()
{
	bIsUnlocked = true;
	UE_LOG(LogTemp, Log, TEXT("Skill %s unlocked"), *SkillName.ToString());
}

bool USkillNodeData::Upgrade()
{
	if (!bIsUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill %s not unlocked"), *SkillName.ToString());
		return false;
	}

	if (SkillLevel >= MaxLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill %s already at max level"), *SkillName.ToString());
		return false;
	}

	SkillLevel++;
	bIsUpgraded = (SkillLevel >= MaxLevel);

	UE_LOG(LogTemp, Log, TEXT("Skill %s upgraded to level %d"), *SkillName.ToString(), SkillLevel);

	return true;
}

bool USkillNodeData::CanUnlock(int32 AvailablePoints, int32 CharacterLevel) const
{
	// 检查技能点
	if (AvailablePoints < RequiredPoints)
	{
		return false;
	}

	// 检查角色等级
	if (CharacterLevel < RequiredCharacterLevel)
	{
		return false;
	}

	// 已解锁则不能再解锁
	if (bIsUnlocked)
	{
		return false;
	}

	return true;
}
