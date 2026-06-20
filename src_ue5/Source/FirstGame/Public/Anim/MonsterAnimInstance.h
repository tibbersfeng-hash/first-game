// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// MonsterAnimInstance — C++ 动画状态机（替代 Blueprint 状态机）

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MonsterAnimInstance.generated.h"

class ABaseEnemy;
class UBlendSpace1D;

/**
 * C++ 动画状态机 — 替代 Blueprint AnimGraph 状态机
 *
 * 状态: Idle → Locomotion → Attack → Hit → Death
 * 混合: 使用 BlendSpace1D 根据 Speed 混合 Idle/Walk/Jog
 */
UCLASS()
class FIRSTGAME_API UMonsterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UMonsterAnimInstance();

	// ─── 动画参数（由 C++ 或蓝图设置） ────────────────────────────────

	/** 移动速度 (cm/s) */
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	float Speed;

	/** 是否正在攻击 */
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	uint32 bIsAttacking : 1;

	/** 是否受击 */
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	uint32 bIsHit : 1;

	/** 是否死亡 */
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	uint32 bIsDead : 1;

	// ─── 动画资源引用 ───────────────────────────────────────────────

	/** 位移混合空间 (Idle/Walk/Jog) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Blendspaces")
	UBlendSpace1D* LocomotionBlendSpace;

	// ─── 核心更新 ───────────────────────────────────────────────────

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeBeginPlay() override;

protected:
	// ─── 状态机逻辑 ─────────────────────────────────────────────────

	/** 当前动画状态 */
	UFUNCTION(BlueprintCallable, Category = "Animation|State")
	FName GetCurrentAnimState() const { return CurrentAnimState; }

	/** 获取当前播放的动画序列 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	UAnimSequence* GetCurrentAnimation() const;

private:
	/** 当前动画状态 */
	FName CurrentAnimState;

	/** 上一帧状态（用于过渡检测） */
	FName PreviousAnimState;

	/** 状态过渡计时器 */
	float StateTransitionTimer;

	/** 最小状态持续时间（防止快速切换） */
	static constexpr float MinStateDuration = 0.1f;

	// ─── 状态处理 ───────────────────────────────────────────────────

	void HandleIdleState(float DeltaSeconds);
	void HandleLocomotionState(float DeltaSeconds);
	void HandleAttackState(float DeltaSeconds);
	void HandleHitState(float DeltaSeconds);
	void HandleDeathState(float DeltaSeconds);

	/** 切换到新状态 */
	void TransitionToState(FName NewState);

	/** 播放动画序列 */
	void PlayAnimation(UAnimSequence* AnimSequence, bool bLooping = false);

	/** 播放混合空间 */
	void PlayBlendSpace(UBlendSpace1D* BlendSpace, float BlendInput);

	// ─── 辅助函数 ───────────────────────────────────────────────────

	/** 获取 owning enemy */
	ABaseEnemy* GetOwningEnemy() const;

	/** 速度阈值 */
	static constexpr float IdleToWalkThreshold = 10.f;
	static constexpr float WalkToJogThreshold = 200.f;
};
