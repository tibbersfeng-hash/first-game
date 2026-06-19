// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// AnimTestActor — 测试用 Actor，循环播放 Idle/Walk/Run 动画

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AnimTestActor.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;

/**
 * 动画测试 Actor
 * 自动循环播放 Idle → Walk → Run 动画
 * 可在编辑器中拖入关卡，指定骨骼网格体和动画即可测试
 */
UCLASS(BlueprintType, Blueprintable)
class FIRSTGAME_API AAnimTestActor : public AActor
{
	GENERATED_BODY()

public:
	AAnimTestActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** 骨骼网格体组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* SkeletalMesh;

	/** 待机动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation",
		meta = (DisplayName = "Idle Animation"))
	TObjectPtr<UAnimSequence> IdleAnim;

	/** 走路动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation",
		meta = (DisplayName = "Walk Animation"))
	TObjectPtr<UAnimSequence> WalkAnim;

	/** 奔跑动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation",
		meta = (DisplayName = "Run Animation"))
	TObjectPtr<UAnimSequence> RunAnim;

	/** 每个动画播放时长（秒），0 = 使用动画原始长度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation",
		meta = (DisplayName = "Override Duration", ClampMin = "0"))
	float AnimDurationOverride;

	/** 切换动画时的混合时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation",
		meta = (DisplayName = "Blend Time", ClampMin = "0"))
	float BlendTime;

	/** 是否自动循环播放 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bAutoPlay;

	/** 动画切换间隔（秒），0 = 播完一个再切下一个 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation",
		meta = (DisplayName = "Switch Interval", ClampMin = "0"))
	float SwitchInterval;

	/** 开始播放动画 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayIdle();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayWalk();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayRun();

	/** 停止所有动画 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void StopAll();

	/** 获取当前播放的动画名 */
	UFUNCTION(BlueprintPure, Category = "Animation")
	FName GetCurrentAnimName() const;

private:
	void PlayAnimSequence(UAnimSequence* Anim);
	void AdvanceToNextAnim();

	FTimerHandle CycleTimerHandle;
	int32 CurrentAnimIndex;
	TArray<UAnimSequence*> AnimQueue;
	FName CurrentAnimName;
};
