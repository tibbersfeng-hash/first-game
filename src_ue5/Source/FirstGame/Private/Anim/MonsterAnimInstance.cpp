// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Anim/MonsterAnimInstance.h"
#include "Characters/BaseEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/BlendSpace1D.h"
#include "Engine/World.h"

UMonsterAnimInstance::UMonsterAnimInstance()
{
	// 默认值
	Speed = 0.f;
	bIsAttacking = false;
	bIsHit = false;
	bIsDead = false;

	CurrentAnimState = NAME_None;
	PreviousAnimState = NAME_None;
	StateTransitionTimer = 0.f;
}

void UMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	UE_LOG(LogTemp, Log, TEXT("MonsterAnimInstance: Initialized"));

	// 尝试自动获取 BlendSpace（如果未手动设置）
	if (!LocomotionBlendSpace)
	{
		// 可以通过 owning enemy 的 EnemyType 来自动加载
		// 这里先留空，需要在 BP 中设置或在 C++ 中根据 EnemyType 加载
	}
}

void UMonsterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	UE_LOG(LogTemp, Log, TEXT("MonsterAnimInstance: BeginPlay"));
}

void UMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 更新速度（从 CharacterMovementComponent 获取）
	if (APawn* Pawn = TryGetPawnOwner())
	{
		if (UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
		{
			Speed = MoveComp->Velocity.Size2D();
		}
	}

	// 状态机更新
	StateTransitionTimer += DeltaSeconds;

	if (CurrentAnimState == NAME_None)
	{
		// 初始状态 → Idle
		TransitionToState(TEXT("Idle"));
	}
	else if (CurrentAnimState == TEXT("Idle"))
	{
		HandleIdleState(DeltaSeconds);
	}
	else if (CurrentAnimState == TEXT("Locomotion"))
	{
		HandleLocomotionState(DeltaSeconds);
	}
	else if (CurrentAnimState == TEXT("Attack"))
	{
		HandleAttackState(DeltaSeconds);
	}
	else if (CurrentAnimState == TEXT("Hit"))
	{
		HandleHitState(DeltaSeconds);
	}
	else if (CurrentAnimState == TEXT("Death"))
	{
		HandleDeathState(DeltaSeconds);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterAnimInstance: Unknown state %s"), *CurrentAnimState.ToString());
	}

	PreviousAnimState = CurrentAnimState;
}

// ─── 状态处理 ──────────────────────────────────────────────────────

void UMonsterAnimInstance::HandleIdleState(float DeltaSeconds)
{
	// 检查是否应该切换到 Locomotion
	if (Speed > IdleToWalkThreshold)
	{
		TransitionToState(TEXT("Locomotion"));
		return;
	}

	// 检查受击
	if (bIsHit)
	{
		TransitionToState(TEXT("Hit"));
		bIsHit = false;  // 重置
		return;
	}

	// 检查死亡
	if (bIsDead)
	{
		TransitionToState(TEXT("Death"));
		return;
	}

	// 播放 Idle 动画（通过 BlendSpace 的 0 速度点）
	if (LocomotionBlendSpace && StateTransitionTimer >= MinStateDuration)
	{
		PlayBlendSpace(LocomotionBlendSpace, 0.f);
	}
}

void UMonsterAnimInstance::HandleLocomotionState(float DeltaSeconds)
{
	// 检查是否应该切换到 Idle
	if (Speed <= IdleToWalkThreshold)
	{
		TransitionToState(TEXT("Idle"));
		return;
	}

	// 检查攻击
	if (bIsAttacking)
	{
		TransitionToState(TEXT("Attack"));
		bIsAttacking = false;  // 重置
		return;
	}

	// 检查受击
	if (bIsHit)
	{
		TransitionToState(TEXT("Hit"));
		bIsHit = false;
		return;
	}

	// 检查死亡
	if (bIsDead)
	{
		TransitionToState(TEXT("Death"));
		return;
	}

	// 播放 BlendSpace（根据速度混合）
	if (LocomotionBlendSpace && StateTransitionTimer >= MinStateDuration)
	{
		// 限制速度到 BlendSpace 范围 (0-600)
		float BlendInput = FMath::Clamp(Speed, 0.f, 600.f);
		PlayBlendSpace(LocomotionBlendSpace, BlendInput);
	}
}

void UMonsterAnimInstance::HandleAttackState(float DeltaSeconds)
{
	// 攻击动画播放完毕后自动回到 Idle 或 Locomotion
	// 这里简化处理：2 秒后自动返回
	if (StateTransitionTimer >= 2.0f)
	{
		if (Speed > IdleToWalkThreshold)
		{
			TransitionToState(TEXT("Locomotion"));
		}
		else
		{
			TransitionToState(TEXT("Idle"));
		}
	}

	// 攻击期间可以被打断（受击优先级更高）
	if (bIsHit)
	{
		TransitionToState(TEXT("Hit"));
		bIsHit = false;
	}
}

void UMonsterAnimInstance::HandleHitState(float DeltaSeconds)
{
	// 受击动画播放完毕后自动返回
	if (StateTransitionTimer >= 0.5f)
	{
		if (bIsDead)
		{
			TransitionToState(TEXT("Death"));
		}
		else if (Speed > IdleToWalkThreshold)
		{
			TransitionToState(TEXT("Locomotion"));
		}
		else
		{
			TransitionToState(TEXT("Idle"));
		}
	}
}

void UMonsterAnimInstance::HandleDeathState(float DeltaSeconds)
{
	// 死亡动画播放完毕后保持死亡状态
	// 不自动切换，由外部逻辑处理（如销毁 Actor）
}

// ─── 状态过渡 ──────────────────────────────────────────────────────

void UMonsterAnimInstance::TransitionToState(FName NewState)
{
	if (NewState == CurrentAnimState)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MonsterAnimInstance: %s → %s"), *CurrentAnimState.ToString(), *NewState.ToString());

	PreviousAnimState = CurrentAnimState;
	CurrentAnimState = NewState;
	StateTransitionTimer = 0.f;

	// 根据状态播放对应动画
	if (NewState == TEXT("Idle"))
	{
		// 通过 BlendSpace 的 0 速度点播放 Idle
		if (LocomotionBlendSpace)
		{
			PlayBlendSpace(LocomotionBlendSpace, 0.f);
		}
	}
	else if (NewState == TEXT("Locomotion"))
	{
		// 通过 BlendSpace 播放（速度由 NativeUpdateAnimation 更新）
	}
	else if (NewState == TEXT("Attack"))
	{
		// 攻击动画需要通过外部设置（AnimMontage 或 AnimSequence）
		UE_LOG(LogTemp, Log, TEXT("MonsterAnimInstance: Attack state entered (play montage externally)"));
	}
	else if (NewState == TEXT("Hit"))
	{
		// 受击动画需要通过外部设置
		UE_LOG(LogTemp, Log, TEXT("MonsterAnimInstance: Hit state entered (play montage externally)"));
	}
	else if (NewState == TEXT("Death"))
	{
		// 死亡动画需要通过外部设置
		UE_LOG(LogTemp, Log, TEXT("MonsterAnimInstance: Death state entered (play montage externally)"));
	}
}

// ─── 辅助函数 ──────────────────────────────────────────────────────

void UMonsterAnimInstance::PlayAnimation(UAnimSequence* AnimSequence, bool bLooping)
{
	if (!AnimSequence)
	{
		return;
	}

	// 使用 Montage 播放
	UAnimMontage* Montage = Cast<UAnimMontage>(AnimSequence);
	if (Montage)
	{
		Montage_Play(Montage);
	}
	else
	{
		// 直接播放序列（不推荐，但作为 fallback）
		UE_LOG(LogTemp, Warning, TEXT("MonsterAnimInstance: No montage for %s"), *AnimSequence->GetName());
	}
}

void UMonsterAnimInstance::PlayBlendSpace(UBlendSpace1D* BlendSpace, float BlendInput)
{
	if (!BlendSpace)
	{
		return;
	}

	// 使用 PlaySlotAnimationAsDynamicMontage 播放 BlendSpace
	// 注意：BlendSpace1D 是 UAnimSequenceBase 的子类
	UAnimSequenceBase* AnimBase = Cast<UAnimSequenceBase>(BlendSpace);
	if (AnimBase)
	{
		PlaySlotAnimationAsDynamicMontage(
			AnimBase,
			FName(TEXT("DefaultSlot")),
			0.2f,  // BlendIn
			0.2f,  // BlendOut
			BlendInput,  // InPlayRate (用于 BlendSpace 输入)
			1,     // LoopCount
			0.f    // BlendInTime
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterAnimInstance: Failed to cast BlendSpace to AnimSequenceBase"));
	}
}

ABaseEnemy* UMonsterAnimInstance::GetOwningEnemy() const
{
	return Cast<ABaseEnemy>(TryGetPawnOwner());
}

UAnimSequence* UMonsterAnimInstance::GetCurrentAnimation() const
{
	// 获取当前播放的动画
	return nullptr;  // TODO: 实现
}
