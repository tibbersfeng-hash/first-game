// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Achievements/AchievementData.h"

UAchievementData::UAchievementData()
{
	AchievementID = NAME_None;
	AchievementName = FText::FromString(TEXT("未命名成就"));
	AchievementType = EAchievementType::Special;
	RequiredProgress = 1;
	bIsUnlocked = false;
	CurrentProgress = 0;
}
