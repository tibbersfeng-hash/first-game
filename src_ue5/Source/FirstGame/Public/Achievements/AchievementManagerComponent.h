// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Achievements/AchievementData.h"
#include "AchievementManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlocked, UAchievementData*, Achievement);

/**
 * 成就管理器组件
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FIRSTGAME_API UAchievementManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAchievementManagerComponent();

	virtual void BeginPlay() override;

	/** 注册成就 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void RegisterAchievement(UAchievementData* Achievement);

	/** 更新成就进度 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void UpdateAchievementProgress(FName AchievementID, int32 Progress);

	/** 解锁成就 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	bool UnlockAchievement(FName AchievementID);

	/** 获取成就 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	UAchievementData* GetAchievement(FName AchievementID) const;

	/** 获取所有成就 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	TArray<UAchievementData*> GetAllAchievements() const;

	/** 获取已解锁成就 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	TArray<UAchievementData*> GetUnlockedAchievements() const;

	/** 成就解锁事件 */
	UPROPERTY(BlueprintAssignable, Category = "Achievements")
	FOnAchievementUnlocked OnAchievementUnlocked;

protected:
	/** 成就列表 */
	UPROPERTY()
	TArray<UAchievementData*> Achievements;

private:
	/** 查找成就 */
	UAchievementData* FindAchievement(FName AchievementID) const;
};
