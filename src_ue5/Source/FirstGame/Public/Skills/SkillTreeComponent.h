// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/SkillNodeData.h"
#include "SkillTreeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUnlocked, USkillNodeData*, Skill);

/**
 * 技能树组件
 * 管理角色的技能树、技能点分配
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FIRSTGAME_API USkillTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillTreeComponent();

	virtual void BeginPlay() override;

	/** 添加技能点 */
	UFUNCTION(BlueprintCallable, Category = "Skills")
	void AddSkillPoints(int32 Points);

	/** 解锁技能 */
	UFUNCTION(BlueprintCallable, Category = "Skills")
	bool UnlockSkill(FName SkillID);

	/** 升级技能 */
	UFUNCTION(BlueprintCallable, Category = "Skills")
	bool UpgradeSkill(FName SkillID);

	/** 获取可用技能点 */
	UFUNCTION(BlueprintCallable, Category = "Skills")
	int32 GetAvailableSkillPoints() const;

	/** 获取技能数据 */
	UFUNCTION(BlueprintCallable, Category = "Skills")
	USkillNodeData* GetSkill(FName SkillID) const;

	/** 获取所有已解锁技能 */
	UFUNCTION(BlueprintCallable, Category = "Skills")
	TArray<USkillNodeData*> GetUnlockedSkills() const;

	/** 技能解锁事件 */
	UPROPERTY(BlueprintAssignable, Category = "Skills")
	FOnSkillUnlocked OnSkillUnlocked;

protected:
	/** 可用技能点 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
	int32 AvailableSkillPoints = 0;

	/** 技能节点列表 */
	UPROPERTY()
	TArray<USkillNodeData*> SkillNodes;

private:
	/** 检查前置技能是否已解锁 */
	bool CheckPrerequisites(USkillNodeData* Skill) const;
};
