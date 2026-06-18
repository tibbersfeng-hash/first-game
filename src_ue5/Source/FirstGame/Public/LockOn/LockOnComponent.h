// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// LockOn Component — 目标锁定系统 (ADR-009)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ILockableTarget.h"
#include "LockOnComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockChanged, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetSwitched, AActor*, OldTarget, AActor*, NewTarget);

/**
 * 目标锁定组件 (ADR-009)
 *
 * 挂在 PlayerCharacter 上，管理战斗中的目标锁定：
 * - 球形+锥形范围检测可锁定目标
 * - 多因素优先级算法 (距离/角度/HP) 自动选择最佳目标
 * - Tab 切换目标、手动释放
 * - 目标丢失/死亡自动重锁定
 * - 通过 OnLockChanged 委托通知 Camera/Combat/HUD
 *
 * 性能优化:
 * - 目标列表每 0.5s 更新一次 (非每帧)
 * - SphereOverlap + 手动角度过滤 (比 LineTrace 快)
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class FIRSTGAME_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULockOnComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// ─── 锁定控制 ─────────────────────────────────────────────────────
	/** 锁定最近/最优目标 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void LockOnNearest();

	/** 切换目标 (bNext=true 切到下一个, false 切到上一个) */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SwitchTarget(bool bNext = true);

	/** 释放锁定 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void ReleaseLock();

	// ─── 查询 ─────────────────────────────────────────────────────────
	/** 当前锁定目标 (可能为 nullptr) */
	UFUNCTION(BlueprintPure, Category = "LockOn")
	AActor* GetCurrentTarget() const;

	/** 是否正在锁定中 */
	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsLockedOn() const;

	/** 获取当前缓存的可锁定目标列表 */
	UFUNCTION(BlueprintPure, Category = "LockOn")
	TArray<AActor*> GetLockableTargets() const;

	// ─── 委托 ─────────────────────────────────────────────────────────
	/** 锁定目标变化时广播 (NewTarget 可能为 nullptr = 释放) */
	UPROPERTY(BlueprintAssignable, Category = "LockOn")
	FOnLockChanged OnLockChanged;

	/** 目标切换时广播 (含旧目标和新目标) */
	UPROPERTY(BlueprintAssignable, Category = "LockOn")
	FOnTargetSwitched OnTargetSwitched;

	// ─── 配置 (蓝图可配) ─────────────────────────────────────────────
	/** 锁定范围 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Range")
	float LockRange = 2000.f;

	/** 锁定视角半角 (度, 总 FOV = 此值×2) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Range")
	float LockFOV = 60.f;

	/** 高度差限制 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Range")
	float LockHeightRange = 1000.f;

	/** 切换防抖延迟 (秒) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Timing")
	float SwitchDelay = 0.2f;

	/** 目标离开 FOV 后多久自动切换 (秒) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Timing")
	float LostDelay = 3.f;

	/** 目标列表更新间隔 (秒) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Performance")
	float TargetCacheInterval = 0.5f;

	// ─── 目标优先级权重 ──────────────────────────────────────────────
	/** HP 权重 (HP 越低分越高) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Priority")
	float WeightHP = 0.4f;

	/** 距离权重 (越近分越高) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Priority")
	float WeightDistance = 0.4f;

	/** 角度权重 (越接近相机中心分越高) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Priority")
	float WeightAngle = 0.2f;

protected:
	virtual void BeginPlay() override;

private:
	// ─── 内部方法 ────────────────────────────────────────────────────
	/** 更新缓存的可锁定目标列表 */
	void UpdateLockableTargets();

	/** 计算目标优先级分数 (0-1, 越高越优先) */
	float CalculateTargetPriority(AActor* Target) const;

	/** 检查目标是否有效 (存活、可锁定、在范围内) */
	bool IsTargetValid(AActor* Target) const;

	/** 检查目标是否在 FOV 锥形内 */
	bool IsTargetInFOV(AActor* Target) const;

	/** 选择最优目标 (按优先级排序) */
	AActor* SelectBestTarget() const;

	/** 选择下一个/上一个目标 (循环列表) */
	AActor* SelectNextTarget(bool bNext) const;

	/** 处理当前目标丢失 */
	void HandleTargetLost();

	/** 设置新的锁定目标 (内部, 会广播委托) */
	void SetLockTarget(AActor* NewTarget);

	// ─── Bug #8 fix: FOV 辅助方法 (消除三处重复) ─────────────────────
	/** 计算目标相对玩家前方的角度 (弧度) */
	float GetAngleFromPlayerForward(const FVector& ToTarget) const;

	/** 获取最大 FOV 半角 (弧度) */
	float GetMaxFOVRad() const { return FMath::DegreesToRadians(LockFOV); }

	/** Bug #2 helper: 缓存玩家位置和前方向量 (每 tick 更新一次) */
	FVector CachedPlayerLocation = FVector::ZeroVector;
	FVector CachedPlayerForward = FVector::ForwardVector;

	// ─── 状态 ────────────────────────────────────────────────────────
	/** 当前锁定目标 (WeakPtr, 死亡自动失效) */
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget;

	/** 缓存的可锁定目标列表 */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> CachedTargets;

	/** 上次切换时间 (防抖) */
	float LastSwitchTime = 0.f;

	/** 目标离开 FOV 的累计时间 */
	float TimeOutOfFOV = 0.f;

	/** 上次缓存更新时间 */
	float LastCacheUpdateTime = 0.f;

	/** 是否已初始化 */
	bool bInitialized = false;

	/** 缓存的玩家引用 */
	UPROPERTY()
	AActor* PlayerOwner = nullptr;
};
