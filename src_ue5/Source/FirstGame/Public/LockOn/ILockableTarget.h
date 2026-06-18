// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// ILockableTarget — 标记可被 LockOn 系统锁定的目标 (ADR-009)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ILockableTarget.generated.h"

/**
 * 可锁定目标接口
 *
 * 所有可以被玩家锁定的 Actor (敌人/Boss) 必须实现此接口。
 * LockOnComponent 通过此接口查询目标状态，无需知道具体类型。
 *
 * 用法:
 *   UCLASS(...) class AMyEnemy : public ACharacter, public ILockableTarget { ... };
 */
UINTERFACE(MinimalAPI, BlueprintType)
class ULockableTarget : public UInterface
{
	GENERATED_BODY()
};

class FIRSTGAME_API ILockableTarget
{
	GENERATED_BODY()

public:
	/** 目标当前 HP 比例 (0=死亡, 1=满血) */
	UFUNCTION(BlueprintNativeEvent, Category = "LockOn")
	float GetHPRatio() const;
	virtual float GetHPRatio_Implementation() const { return 1.f; }

	/** 是否可以被锁定 (存活且未被锁定免疫) */
	UFUNCTION(BlueprintNativeEvent, Category = "LockOn")
	bool IsLockable() const;
	virtual bool IsLockable_Implementation() const { return true; }

	/** 锁定优先级权重 (越高越容易被选中, 默认 1.0) */
	UFUNCTION(BlueprintNativeEvent, Category = "LockOn")
	float GetLockPriority() const;
	virtual float GetLockPriority_Implementation() const { return 1.f; }
};
