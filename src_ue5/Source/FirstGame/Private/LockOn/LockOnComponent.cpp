// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// LockOn Component — 目标锁定系统实现 (ADR-009)

#include "LockOn/LockOnComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.f; // 每帧 tick (用于丢失检测)
}

void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	// 缓存玩家引用
	if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		PlayerOwner = PawnOwner;
	}

	bInitialized = (PlayerOwner != nullptr);
	if (!bInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("LockOnComponent: Owner is not a Pawn, lock-on disabled."));
	}

	// 初始扫描
	UpdateLockableTargets();
}

void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	if (IsRegistered()) { Super::TickComponent(DeltaTime, TickType, ThisTickFunction); }

	if (!bInitialized) { return; }

	const float WorldTime = GetWorld()->GetTimeSeconds();

	// Bug #7 fix: 缓存玩家变换, 避免每目标重复获取
	if (PlayerOwner)
	{
		CachedPlayerLocation = PlayerOwner->GetActorLocation();
		CachedPlayerForward = PlayerOwner->GetActorForwardVector();
	}

	// 定期更新目标缓存
	if (WorldTime - LastCacheUpdateTime >= TargetCacheInterval)
	{
		UpdateLockableTargets();
		LastCacheUpdateTime = WorldTime;
	}

	// 如果有锁定目标，检测是否丢失
	if (CurrentTarget.IsValid())
	{
		AActor* Target = CurrentTarget.Get();

		// 目标死亡 → 立即处理
		if (!IsTargetValid(Target))
		{
			HandleTargetLost();
			return;
		}

		// 目标离开 FOV → 累计时间
		if (!IsTargetInFOV(Target))
		{
			TimeOutOfFOV += DeltaTime;
			if (TimeOutOfFOV >= LostDelay)
			{
				HandleTargetLost();
			}
		}
		else
		{
			TimeOutOfFOV = 0.f;
		}
	}
}

// ── 锁定控制 ─────────────────────────────────────────────────────────

void ULockOnComponent::LockOnNearest()
{
	if (!bInitialized) { return; }

	AActor* BestTarget = SelectBestTarget();
	if (BestTarget)
	{
		SetLockTarget(BestTarget);
		UE_LOG(LogTemp, Log, TEXT("LockOn: Locked onto %s"), *BestTarget->GetActorLabel());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("LockOn: No lockable targets in range."));
	}
}

void ULockOnComponent::SwitchTarget(bool bNext)
{
	if (!bInitialized) { return; }

	// 防抖
	const float WorldTime = GetWorld()->GetTimeSeconds();
	if (WorldTime - LastSwitchTime < SwitchDelay) { return; }
	LastSwitchTime = WorldTime;

	AActor* NextTarget = SelectNextTarget(bNext);
	if (NextTarget)
	{
		AActor* OldTarget = CurrentTarget.IsValid() ? CurrentTarget.Get() : nullptr;
		SetLockTarget(NextTarget);
		OnTargetSwitched.Broadcast(OldTarget, NextTarget);
		UE_LOG(LogTemp, Log, TEXT("LockOn: Switched from %s to %s"),
			OldTarget ? *OldTarget->GetActorLabel() : TEXT("None"),
			*NextTarget->GetActorLabel());
	}
}

void ULockOnComponent::ReleaseLock()
{
	if (!CurrentTarget.IsValid()) { return; }

	AActor* OldTarget = CurrentTarget.Get();
	SetLockTarget(nullptr);
	UE_LOG(LogTemp, Log, TEXT("LockOn: Released lock on %s"), *OldTarget->GetActorLabel());
}

// ─── 查询 ─────────────────────────────────────────────────────────────

AActor* ULockOnComponent::GetCurrentTarget() const
{
	return CurrentTarget.IsValid() ? CurrentTarget.Get() : nullptr;
}

bool ULockOnComponent::IsLockedOn() const
{
	return CurrentTarget.IsValid() && IsTargetValid(CurrentTarget.Get());
}

TArray<AActor*> ULockOnComponent::GetLockableTargets() const
{
	TArray<AActor*> Result;
	for (const TWeakObjectPtr<AActor>& WeakTarget : CachedTargets)
	{
		if (WeakTarget.IsValid())
		{
			Result.Add(WeakTarget.Get());
		}
	}
	return Result;
}

// ─── 内部方法 ─────────────────────────────────────────────────────────

void ULockOnComponent::UpdateLockableTargets()
{
	if (!bInitialized || !PlayerOwner) { return; }

	CachedTargets.Empty();

	const FVector PlayerLoc = PlayerOwner->GetActorLocation();
	TArray<FOverlapResult> OverlapResults;

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(LockRange);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LockOn), false, PlayerOwner);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults, PlayerLoc, FQuat::Identity,
		ECC_Pawn, SphereShape, QueryParams);

	if (!bHit) { return; }

	const FVector PlayerForward = PlayerOwner->GetActorForwardVector();
	const float MaxFOVRad = FMath::DegreesToRadians(LockFOV);
	const FVector PlayerUp = PlayerOwner->GetActorUpVector();

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor || HitActor == PlayerOwner) { continue; }

		// 必须实现 ILockableTarget 接口
		if (!HitActor->GetClass()->ImplementsInterface(ULockableTarget::StaticClass()))
		{
			continue;
		}

		// 检查是否可锁定
		if (!ILockableTarget::Execute_IsLockable(HitActor)) { continue; }

		// 距离检查
		const FVector ToTarget = HitActor->GetActorLocation() - PlayerLoc;
		const float Distance = ToTarget.Size();
		if (Distance > LockRange) { continue; }

		// 高度差检查
		const float HeightDiff = FMath::Abs(FVector::DotProduct(ToTarget, PlayerUp));
		if (HeightDiff > LockHeightRange) { continue; }

		// FOV 锥形检查 (Bug #8: 使用共享辅助方法)
		const float FOVAngle = GetAngleFromPlayerForward(ToTarget);
		if (FOVAngle > GetMaxFOVRad()) { continue; }

		CachedTargets.Add(HitActor);
	}
}

float ULockOnComponent::CalculateTargetPriority(AActor* Target) const
{
	if (!Target || !PlayerOwner) { return 0.f; }

	const FVector PlayerLoc = PlayerOwner->GetActorLocation();
	const FVector TargetLoc = Target->GetActorLocation();

	// 1. 距离分数 (0-1, 越近越高)
	const float Distance = FVector::Dist(PlayerLoc, TargetLoc);
	const float DistanceScore = 1.f - FMath::Clamp(Distance / LockRange, 0.f, 1.f);

	// 2. 角度分数 (0-1, 越接近玩家前方越高) (Bug #8: 使用缓存变换 + 共享辅助)
	const FVector ToTarget = TargetLoc - CachedPlayerLocation;
	const float Angle = GetAngleFromPlayerForward(ToTarget);
	const float MaxFOV = GetMaxFOVRad();
	const float AngleScore = MaxFOV > 0.f
		? 1.f - FMath::Clamp(Angle / MaxFOV, 0.f, 1.f)
		: 0.f;

	// 3. HP 分数 (0-1, HP 越低越高 — 优先收割残血)
	const float HPRatio = ILockableTarget::Execute_GetHPRatio(Target);
	const float HPScore = 1.f - HPRatio;

	// 4. 接口自定义优先级权重
	const float CustomPriority = ILockableTarget::Execute_GetLockPriority(Target);

	// 加权综合
	return (DistanceScore * WeightDistance
		+ AngleScore * WeightAngle
		+ HPScore * WeightHP) * CustomPriority;
}

bool ULockOnComponent::IsTargetValid(AActor* Target) const
{
	if (!Target) { return false; }
	if (!Target->IsValidLowLevel()) { return false; }

	// 必须实现接口且返回 IsLockable
	if (!Target->GetClass()->ImplementsInterface(ULockableTarget::StaticClass()))
	{
		return false;
	}

	return ILockableTarget::Execute_IsLockable(Target);
}

bool ULockOnComponent::IsTargetInFOV(AActor* Target) const
{
	if (!Target || !PlayerOwner) { return false; }

	const FVector ToTarget = Target->GetActorLocation() - CachedPlayerLocation;
	const float Angle = GetAngleFromPlayerForward(ToTarget);
	return Angle <= GetMaxFOVRad();
}

AActor* ULockOnComponent::SelectBestTarget() const
{
	AActor* BestTarget = nullptr;
	float BestScore = -1.f;

	for (const TWeakObjectPtr<AActor>& WeakTarget : CachedTargets)
	{
		if (!WeakTarget.IsValid()) { continue; }

		AActor* Target = WeakTarget.Get();

		// Bug #3 fix: 过滤已死亡/无效目标, 防止从过期缓存中选择死目标
		if (!IsTargetValid(Target)) { continue; }

		const float Score = CalculateTargetPriority(Target);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

AActor* ULockOnComponent::SelectNextTarget(bool bNext) const
{
	if (CachedTargets.Num() == 0) { return nullptr; }

	// 找到当前目标在列表中的位置
	int32 CurrentIndex = -1;
	if (CurrentTarget.IsValid())
	{
		for (int32 i = 0; i < CachedTargets.Num(); i++)
		{
			if (CachedTargets[i].IsValid() && CachedTargets[i].Get() == CurrentTarget.Get())
			{
				CurrentIndex = i;
				break;
			}
		}
	}

	// 按优先级排序后循环选择
	struct FTargetScore
	{
		AActor* Actor;
		float Score;
	};

	TArray<FTargetScore> ScoredTargets;
	for (const TWeakObjectPtr<AActor>& WeakTarget : CachedTargets)
	{
		if (!WeakTarget.IsValid()) { continue; }
		FTargetScore TS;
		TS.Actor = WeakTarget.Get();
		TS.Score = CalculateTargetPriority(TS.Actor);
		ScoredTargets.Add(TS);
	}

	// 按分数降序排序
	ScoredTargets.Sort([](const FTargetScore& A, const FTargetScore& B)
	{
		return A.Score > B.Score;
	});

	if (ScoredTargets.Num() == 0) { return nullptr; }

	// 如果当前没有锁定，返回最优
	if (CurrentIndex < 0)
	{
		return bNext ? ScoredTargets[0].Actor : ScoredTargets.Last().Actor;
	}

	// 在排序列表中找到当前目标的位置
	int32 SortedIndex = -1;
	for (int32 i = 0; i < ScoredTargets.Num(); i++)
	{
		if (ScoredTargets[i].Actor == CurrentTarget.Get())
		{
			SortedIndex = i;
			break;
		}
	}

	if (SortedIndex < 0)
	{
		return bNext ? ScoredTargets[0].Actor : ScoredTargets.Last().Actor;
	}

	// 循环切换
	int32 NextIndex = bNext
		? (SortedIndex + 1) % ScoredTargets.Num()
		: (SortedIndex - 1 + ScoredTargets.Num()) % ScoredTargets.Num();

	return ScoredTargets[NextIndex].Actor;
}

void ULockOnComponent::HandleTargetLost()
{
	if (!bInitialized) { return; }

	// 尝试切换到次优目标
	AActor* NextTarget = SelectBestTarget();
	if (NextTarget && NextTarget != CurrentTarget.Get())
	{
		AActor* OldTarget = CurrentTarget.IsValid() ? CurrentTarget.Get() : nullptr;
		SetLockTarget(NextTarget);
		OnTargetSwitched.Broadcast(OldTarget, NextTarget);
		UE_LOG(LogTemp, Log, TEXT("LockOn: Auto-switched to %s after target lost"),
			*NextTarget->GetActorLabel());
	}
	else
	{
		AActor* OldTarget = CurrentTarget.IsValid() ? CurrentTarget.Get() : nullptr;
		SetLockTarget(nullptr);
		UE_LOG(LogTemp, Log, TEXT("LockOn: Target lost, no replacement. Released lock."));
	}

	TimeOutOfFOV = 0.f;
}

void ULockOnComponent::SetLockTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
	OnLockChanged.Broadcast(NewTarget);
}

// ─── Bug #8 fix: FOV 辅助方法 ──────────────────────────────────────

float ULockOnComponent::GetAngleFromPlayerForward(const FVector& ToTarget) const
{
	const FVector ToTargetNorm = ToTarget.GetSafeNormal();
	const float DotProduct = FVector::DotProduct(ToTargetNorm, CachedPlayerForward);
	return FMath::Acos(FMath::Clamp(DotProduct, -1.f, 1.f));
}
