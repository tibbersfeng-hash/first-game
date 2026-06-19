// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "AnimTestActor.h"
#include "FirstGame.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAnimTestActor::AAnimTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	RootComponent = SkeletalMesh;

	// "使用动画资产"模式
	SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	// 禁用物理模拟，防止 PIE 时飞天
	SkeletalMesh->SetSimulatePhysics(false);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AnimDurationOverride = 0.f;
	BlendTime = 0.2f;
	bAutoPlay = true;
	SwitchInterval = 0.f;
	CurrentAnimIndex = 0;
	CurrentAnimName = "None";
}

void AAnimTestActor::BeginPlay()
{
	Super::BeginPlay();

	AnimQueue.Empty();
	if (IdleAnim)  AnimQueue.Add(IdleAnim);
	if (WalkAnim)  AnimQueue.Add(WalkAnim);
	if (RunAnim)   AnimQueue.Add(RunAnim);

	if (AnimQueue.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimTestActor] No animations assigned!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[AnimTestActor] BeginPlay: %d anim(s)"), AnimQueue.Num());

	if (bAutoPlay && !HasAnyFlags(RF_ClassDefaultObject))
	{
		PlayAnimSequence(AnimQueue[0]);

		if (SwitchInterval > 0.f)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				World->GetTimerManager().SetTimer(
					CycleTimerHandle,
					this,
					&AAnimTestActor::AdvanceToNextAnim,
					SwitchInterval,
					true
				);
				UE_LOG(LogTemp, Log, TEXT("[AnimTestActor] Timer set: %.1fs"), SwitchInterval);
			}
		}
	}
}

void AAnimTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 先清定时器（防止回调访问已销毁对象）
	if (CycleTimerHandle.IsValid())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(CycleTimerHandle);
		}
		CycleTimerHandle.Invalidate();
	}

	// 清空动画队列（防止悬空指针）
	AnimQueue.Empty();

	Super::EndPlay(EndPlayReason);
}

void AAnimTestActor::PlayAnimSequence(UAnimSequence* Anim)
{
	if (!Anim || !SkeletalMesh || !IsValid(SkeletalMesh))
	{
		return;
	}

	// 安全检查：确保 AnimInstance 有效
	UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance();
	if (!AnimInst || !IsValid(AnimInst))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimTestActor] No valid AnimInstance"));
		return;
	}

	SkeletalMesh->PlayAnimation(Anim, true);
	CurrentAnimName = FName(*Anim->GetName());
	UE_LOG(LogTemp, Log, TEXT("[AnimTestActor] Playing: %s"), *CurrentAnimName.ToString());
}

void AAnimTestActor::AdvanceToNextAnim()
{
	// 安全检查：PIE 结束时可能 Actor 正在被销毁
	if (!IsValid(this) || AnimQueue.IsEmpty())
	{
		return;
	}

	CurrentAnimIndex = (CurrentAnimIndex + 1) % AnimQueue.Num();
	UAnimSequence* NextAnim = AnimQueue[CurrentAnimIndex];

	if (NextAnim && IsValid(NextAnim))
	{
		UE_LOG(LogTemp, Log, TEXT("[AnimTestActor] Switch: %s"), *NextAnim->GetName());
		PlayAnimSequence(NextAnim);
	}
}

void AAnimTestActor::PlayIdle()  { if (IdleAnim)  { CurrentAnimIndex = 0; PlayAnimSequence(IdleAnim); } }
void AAnimTestActor::PlayWalk()  { if (WalkAnim)  { CurrentAnimIndex = 1; PlayAnimSequence(WalkAnim); } }
void AAnimTestActor::PlayRun()   { if (RunAnim)   { CurrentAnimIndex = 2; PlayAnimSequence(RunAnim); } }

void AAnimTestActor::StopAll()
{
	if (SkeletalMesh && IsValid(SkeletalMesh))
	{
		UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance();
		if (AnimInst && IsValid(AnimInst))
		{
			AnimInst->StopAllMontages(0.f);
		}
	}
	CurrentAnimName = "None";
}

FName AAnimTestActor::GetCurrentAnimName() const { return CurrentAnimName; }
