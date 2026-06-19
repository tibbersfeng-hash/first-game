// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "AnimTestActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"

AAnimTestActor::AAnimTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建骨骼网格体组件
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	RootComponent = SkeletalMesh;

	// 关键：设置为"使用动画资产"模式，这样 PlayAnimation() 才能直接播放
	// EAnimationMode::AnimationSingleNode = "Use Animation Asset"
	SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	// 默认值
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

	// 构建动画队列：Idle → Walk → Run
	AnimQueue.Empty();
	if (IdleAnim)  AnimQueue.Add(IdleAnim);
	if (WalkAnim)  AnimQueue.Add(WalkAnim);
	if (RunAnim)   AnimQueue.Add(RunAnim);

	if (AnimQueue.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimTestActor] No animations assigned! Set Idle/Walk/Run in Details panel."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[AnimTestActor] BeginPlay: %d animation(s) queued"), AnimQueue.Num());

	for (int32 i = 0; i < AnimQueue.Num(); ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("[AnimTestActor]   [%d] %s (%.2fs)"),
			i,
			AnimQueue[i] ? *AnimQueue[i]->GetName() : TEXT("null"),
			AnimQueue[i] ? AnimQueue[i]->GetPlayLength() : 0.f);
	}

	if (bAutoPlay)
	{
		PlayAnimSequence(AnimQueue[0]);

		if (SwitchInterval > 0.f)
		{
			GetWorldTimerManager().SetTimer(
				CycleTimerHandle,
				this,
				&AAnimTestActor::AdvanceToNextAnim,
				SwitchInterval,
				true
			);
			UE_LOG(LogTemp, Log, TEXT("[AnimTestActor] Auto-switch every %.1fs"), SwitchInterval);
		}
	}
}

void AAnimTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (CycleTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(CycleTimerHandle);
	}

	StopAll();
}

void AAnimTestActor::PlayAnimSequence(UAnimSequence* Anim)
{
	if (!Anim || !SkeletalMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimTestActor] PlayAnimSequence: null anim or mesh"));
		return;
	}

	// PlayAnimation 自动替换当前动画
	SkeletalMesh->PlayAnimation(Anim, true);
	CurrentAnimName = FName(*Anim->GetName());
	UE_LOG(LogTemp, Log, TEXT("[AnimTestActor] Playing: %s (looping)"), *CurrentAnimName.ToString());
}

void AAnimTestActor::AdvanceToNextAnim()
{
	if (AnimQueue.IsEmpty()) return;

	CurrentAnimIndex = (CurrentAnimIndex + 1) % AnimQueue.Num();
	UAnimSequence* NextAnim = AnimQueue[CurrentAnimIndex];

	UE_LOG(LogTemp, Log, TEXT("[AnimTestActor] Switching to: %s"),
		NextAnim ? *NextAnim->GetName() : TEXT("null"));
	PlayAnimSequence(NextAnim);
}

void AAnimTestActor::PlayIdle()
{
	if (IdleAnim) { CurrentAnimIndex = 0; PlayAnimSequence(IdleAnim); }
}

void AAnimTestActor::PlayWalk()
{
	if (WalkAnim) { CurrentAnimIndex = 1; PlayAnimSequence(WalkAnim); }
}

void AAnimTestActor::PlayRun()
{
	if (RunAnim) { CurrentAnimIndex = 2; PlayAnimSequence(RunAnim); }
}

void AAnimTestActor::StopAll()
{
	if (SkeletalMesh)
	{
		UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance();
		if (AnimInst)
		{
			AnimInst->StopAllMontages(0.f);
		}
	}
	CurrentAnimName = "None";
}

FName AAnimTestActor::GetCurrentAnimName() const
{
	return CurrentAnimName;
}
