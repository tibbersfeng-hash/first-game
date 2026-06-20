// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillNodeData.generated.h"

/**
 * 技能类型
 */
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Active      UMETA(DisplayName = "主动技能"),
	Passive     UMETA(DisplayName = "被动技能"),
	Ultimate    UMETA(DisplayName = "必杀技")
};

/**
 * 技能节点数据
 * 定义技能树中的单个技能节点
 */
UCLASS(BlueprintType, Blueprintable)
class FIRSTGAME_API USkillNodeData : public UDataAsset
{
	GENERATED_BODY()

public:
	USkillNodeData();

	/** 技能 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName SkillID;

	/** 技能名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText SkillName;

	/** 技能描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText Description;

	/** 技能类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	ESkillType SkillType;

	/** 技能等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	int32 SkillLevel = 1;

	/** 最大等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	int32 MaxLevel = 5;

	/** 解锁所需技能点 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredPoints = 1;

	/** 前置技能 ID 列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> PrerequisiteSkills;

	/** 解锁所需角色等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredCharacterLevel = 1;

	/** 技能图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UTexture2D* Icon;

	/** 是否已解锁 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bIsUnlocked = false;

	/** 是否已升级 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bIsUpgraded = false;

	/** 解锁技能 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void Unlock();

	/** 升级技能 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool Upgrade();

	/** 检查是否可以解锁 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool CanUnlock(int32 AvailablePoints, int32 CharacterLevel) const;
};
