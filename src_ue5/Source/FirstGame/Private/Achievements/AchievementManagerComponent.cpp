// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Achievements/AchievementManagerComponent.h"
#include "FirstGame.h"

UAchievementManagerComponent::UAchievementManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAchievementManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("AchievementManager: Initialized"));
}

void UAchievementManagerComponent::RegisterAchievement(UAchievementData* Achievement)
{
	if (!Achievement)
	{
		return;
	}

	// 检查是否已注册
	if (FindAchievement(Achievement->AchievementID))
	{
		UE_LOG(LogTemp, Warning, TEXT("AchievementManager: Achievement %s already registered"), *Achievement->AchievementID.ToString());
		return;
	}

	Achievements.Add(Achievement);
	UE_LOG(LogTemp, Log, TEXT("AchievementManager: Registered achievement %s"), *Achievement->AchievementName.ToString());
}

void UAchievementManagerComponent::UpdateAchievementProgress(FName AchievementID, int32 Progress)
{
	UAchievementData* Achievement = FindAchievement(AchievementID);
	if (!Achievement)
	{
		return;
	}

	if (Achievement->bIsUnlocked)
	{
		return;
	}

	Achievement->CurrentProgress += Progress;

	UE_LOG(LogTemp, Log, TEXT("AchievementManager: Updated %s progress to %d/%d"),
		*Achievement->AchievementID.ToString(), Achievement->CurrentProgress, Achievement->RequiredProgress);

	// 检查是否达到解锁条件
	if (Achievement->CurrentProgress >= Achievement->RequiredProgress)
	{
		UnlockAchievement(AchievementID);
	}
}

bool UAchievementManagerComponent::UnlockAchievement(FName AchievementID)
{
	UAchievementData* Achievement = FindAchievement(AchievementID);
	if (!Achievement)
	{
		return false;
	}

	if (Achievement->bIsUnlocked)
	{
		return false;
	}

	Achievement->bIsUnlocked = true;
	Achievement->UnlockTime = FDateTime::Now();

	UE_LOG(LogTemp, Log, TEXT("AchievementManager: Unlocked achievement %s"), *Achievement->AchievementName.ToString());

	// 广播事件
	OnAchievementUnlocked.Broadcast(Achievement);

	return true;
}

UAchievementData* UAchievementManagerComponent::GetAchievement(FName AchievementID) const
{
	return FindAchievement(AchievementID);
}

TArray<UAchievementData*> UAchievementManagerComponent::GetAllAchievements() const
{
	return Achievements;
}

TArray<UAchievementData*> UAchievementManagerComponent::GetUnlockedAchievements() const
{
	TArray<UAchievementData*> Unlocked;
	for (UAchievementData* Achievement : Achievements)
	{
		if (Achievement && Achievement->bIsUnlocked)
		{
			Unlocked.Add(Achievement);
		}
	}
	return Unlocked;
}

UAchievementData* UAchievementManagerComponent::FindAchievement(FName AchievementID) const
{
	for (UAchievementData* Achievement : Achievements)
	{
		if (Achievement && Achievement->AchievementID == AchievementID)
		{
			return Achievement;
		}
	}
	return nullptr;
}
