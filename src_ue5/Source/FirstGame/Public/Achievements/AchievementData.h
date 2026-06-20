// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AchievementData.generated.h"

/**
 * 成就类型
 */
UENUM(BlueprintType)
enum class EAchievementType : uint8
{
	Combat      UMETA(DisplayName = "战斗"),
	Exploration UMETA(DisplayName = "探索"),
	Collection  UMETA(DisplayName = "收集"),
	Social      UMETA(DisplayName = "社交"),
	Special     UMETA(DisplayName = "特殊")
};

/**
 * 成就数据
 */
UCLASS(BlueprintType, Blueprintable)
class FIRSTGAME_API UAchievementData : public UDataAsset
{
	GENERATED_BODY()

public:
	UAchievementData();

	/** 成就 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName AchievementID;

	/** 成就名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText AchievementName;

	/** 成就描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText Description;

	/** 成就类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	EAchievementType AchievementType;

	/** 成就图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UTexture2D* Icon;

	/** 解锁条件描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FText UnlockCondition;

	/** 需要的进度（如击杀 100 个敌人） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredProgress = 1;

	/** 奖励类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FText RewardDescription;

	/** 是否已解锁 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bIsUnlocked = false;

	/** 当前进度 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	int32 CurrentProgress = 0;

	/** 解锁时间 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	FDateTime UnlockTime;
};
