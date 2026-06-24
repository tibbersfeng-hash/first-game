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

	UE_LOG(LogTemp, Log, TEXT("MonsterAnimInstance: Initialized for %s"), *GetName());

	// 自动获取 owning enemy 并配置动画
	ABaseEnemy* Enemy = GetOwningEnemy();
	if (!Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterAnimInstance: No owning enemy found"));
		return;
	}

	// 根据 EnemyType 加载 BlendSpace 和动画
	// 如果 LocomotionBlendSpace 未设置，尝试自动加载
	if (!LocomotionBlendSpace)
	{
		// 尝试加载每个怪物的 Locomotion BlendSpace
		FString BlendSpacePath;
		uint8 EnemyTypeVal = Enemy->GetEnemyTypeValue();

		switch (EnemyTypeVal)
		{
		case 0: // CandyZombie
			BlendSpacePath = TEXT("/Game/Monsters/CandyZombie/BlendSpaces/BS_CandyZombie_Locomotion");
			break;
		case 1: // Gingerbread
			BlendSpacePath = TEXT("/Game/Monsters/Gingerbread/BlendSpaces/BS_Gingerbread_Locomotion");
			break;
		case 2: // ShadowNinja
			BlendSpacePath = TEXT("/Game/Monsters/ShadowNinja/BlendSpaces/BS_ShadowNinja_Locomotion");
			break;
		case 3: // ArmoredGum
			BlendSpacePath = TEXT("/Game/Monsters/ArmoredGum/BlendSpaces/BS_ArmoredGum_Locomotion");
			break;
		}

		if (!BlendSpacePath.IsEmpty())
		{
			LocomotionBlendSpace = LoadObject<UBlendSpace1D>(nullptr, *BlendSpacePath);
			if (LocomotionBlendSpace)
			{
				UE_LOG(LogTemp, Log, TEXT("MonsterAnimInstance: 自动加载 BlendSpace %s"), *BlendSpacePath);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("MonsterAnimInstance: 无法加载 BlendSpace %s"), *BlendSpacePath);
			}
		}
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

	// 使用 PlaySlotAnimationAsDynamicMontage 播放动画序列
	PlaySlotAnimationAsDynamicMontage(
		AnimSequence,
		FName(TEXT("DefaultSlot")),
		0.2f,  // BlendIn
		0.2f,  // BlendOut
		1.0f,  // PlayRate
		1,     // LoopCount
		bLooping ? -1.f : 0.f  // BlendInTime (-1 = infinite loop)
	);
}

void UMonsterAnimInstance::PlayBlendSpace(UBlendSpace1D* BlendSpace, float BlendInput)
{
	if (!BlendSpace)
	{
		return;
	}

	// UBlendSpace1D 继承自 UAnimSequenceBase
	// 使用 Cast 转换后播放
	UAnimSequenceBase* AnimBase = Cast<UAnimSequenceBase>(BlendSpace);
	if (AnimBase)
	{
		PlaySlotAnimationAsDynamicMontage(
			AnimBase,
			FName(TEXT("DefaultSlot")),
			0.2f,  // BlendIn
			0.2f,  // BlendOut
			1.0f,  // PlayRate
			1,     // LoopCount
			-1.f   // BlendOutTriggerTime
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterAnimInstance: Cannot cast BlendSpace to AnimSequenceBase"));
	}
}

ABaseEnemy* UMonsterAnimInstance::GetOwningEnemy() const
{
	return Cast<ABaseEnemy>(TryGetPawnOwner());
}

UAnimSequence* UMonsterAnimInstance::GetCurrentAnimation() const
{
	// 根据当前状态返回对应的动画序列
	ABaseEnemy* Enemy = const_cast<UMonsterAnimInstance*>(this)->GetOwningEnemy();
	if (!Enemy) return nullptr;

	uint8 EnemyType = Enemy->GetEnemyTypeValue();
	FString BasePath;

	switch (EnemyType)
	{
	case 0: BasePath = TEXT("/Game/Monsters/CandyZombie/Animations"); break;
	case 1: BasePath = TEXT("/Game/Monsters/Gingerbread/Animations"); break;
	case 2: BasePath = TEXT("/Game/Monsters/ShadowNinja/Animations"); break;
	case 3: BasePath = TEXT("/Game/Monsters/ArmoredGum/Animations"); break;
	default: return nullptr;
	}

	// 根据状态返回动画
	if (CurrentAnimState == TEXT("Idle"))
	{
		return LoadObject<UAnimSequence>(nullptr, *(BasePath + TEXT("/AM_Idle")));
	}
	else if (CurrentAnimState == TEXT("Attack"))
	{
		return LoadObject<UAnimSequence>(nullptr, *(BasePath + TEXT("/AM_Attack")));
	}
	else if (CurrentAnimState == TEXT("Hit"))
	{
		return LoadObject<UAnimSequence>(nullptr, *(BasePath + TEXT("/AM_Hit")));
	}
	else if (CurrentAnimState == TEXT("Death"))
	{
		return LoadObject<UAnimSequence>(nullptr, *(BasePath + TEXT("/AM_Death")));
	}

	// Locomotion 使用 BlendSpace，不返回单个序列
	return nullptr;
}
