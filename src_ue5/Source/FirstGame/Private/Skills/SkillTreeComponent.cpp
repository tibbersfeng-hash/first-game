// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Skills/SkillTreeComponent.h"
#include "FirstGame.h"

USkillTreeComponent::USkillTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	AvailableSkillPoints = 0;
}

void USkillTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("SkillTree: Initialized with %d skill nodes"), SkillNodes.Num());
}

void USkillTreeComponent::AddSkillPoints(int32 Points)
{
	AvailableSkillPoints += Points;
	UE_LOG(LogTemp, Log, TEXT("SkillTree: Added %d skill points, total: %d"), Points, AvailableSkillPoints);
}

bool USkillTreeComponent::UnlockSkill(FName SkillID)
{
	USkillNodeData* Skill = GetSkill(SkillID);
	if (!Skill)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillTree: Skill %s not found"), *SkillID.ToString());
		return false;
	}

	// 检查是否可以解锁
	if (!Skill->CanUnlock(AvailableSkillPoints, 1)) // 角色等级暂时用 1
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillTree: Cannot unlock %s"), *Skill->SkillName.ToString());
		return false;
	}

	// 检查前置技能
	if (!CheckPrerequisites(Skill))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillTree: Prerequisites not met for %s"), *Skill->SkillName.ToString());
		return false;
	}

	// 解锁技能
	Skill->Unlock();
	AvailableSkillPoints -= Skill->RequiredPoints;

	UE_LOG(LogTemp, Log, TEXT("SkillTree: Unlocked %s, remaining points: %d"),
		*Skill->SkillName.ToString(), AvailableSkillPoints);

	// 广播事件
	OnSkillUnlocked.Broadcast(Skill);

	return true;
}

bool USkillTreeComponent::UpgradeSkill(FName SkillID)
{
	USkillNodeData* Skill = GetSkill(SkillID);
	if (!Skill)
	{
		return false;
	}

	if (!Skill->bIsUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillTree: Skill %s not unlocked"), *Skill->SkillName.ToString());
		return false;
	}

	bool Result = Skill->Upgrade();
	if (Result)
	{
		UE_LOG(LogTemp, Log, TEXT("SkillTree: Upgraded %s to level %d"),
			*Skill->SkillName.ToString(), Skill->SkillLevel);
	}

	return Result;
}

int32 USkillTreeComponent::GetAvailableSkillPoints() const
{
	return AvailableSkillPoints;
}

USkillNodeData* USkillTreeComponent::GetSkill(FName SkillID) const
{
	for (USkillNodeData* Skill : SkillNodes)
	{
		if (Skill && Skill->SkillID == SkillID)
		{
			return Skill;
		}
	}
	return nullptr;
}

TArray<USkillNodeData*> USkillTreeComponent::GetUnlockedSkills() const
{
	TArray<USkillNodeData*> Unlocked;
	for (USkillNodeData* Skill : SkillNodes)
	{
		if (Skill && Skill->bIsUnlocked)
		{
			Unlocked.Add(Skill);
		}
	}
	return Unlocked;
}

bool USkillTreeComponent::CheckPrerequisites(USkillNodeData* Skill) const
{
	if (Skill->PrerequisiteSkills.Num() == 0)
	{
		return true;
	}

	for (const FName& PrereqID : Skill->PrerequisiteSkills)
	{
		USkillNodeData* PrereqSkill = GetSkill(PrereqID);
		if (!PrereqSkill || !PrereqSkill->bIsUnlocked)
		{
			return false;
		}
	}

	return true;
}
