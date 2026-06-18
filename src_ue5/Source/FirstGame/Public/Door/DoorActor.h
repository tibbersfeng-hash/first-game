// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Door Actor — 房间之间的门 (通关后开启)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorStateChanged, class ADoorActor*, Door);

/**
 * 房间门 — 阻塞玩家直到房间被 Clear
 *
 * 用法:
 *   1. LevelBuilder 在房间之间 Spawn 门
 *   2. DungeonRoom.ExitDoor 指向该门
 *   3. DungeonRoom.ClearRoom() 调用 Door->Open()
 *   4. 玩家穿过门进入下一房间
 *
 * 默认状态: 关闭 (阻挡玩家)
 * 开启后: 碰撞关闭, 玩家可通过
 */
UCLASS(Blueprintable)
class FIRSTGAME_API ADoorActor : public AActor
{
	GENERATED_BODY()

public:
	ADoorActor();

	/** 门是否已开启 */
	UPROPERTY(BlueprintReadOnly, Category = "Door")
	bool bIsOpen = false;

	/** 开门 (关闭碰撞, 播放开门动画) */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void Open();

	/** 关门 (开启碰撞) */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void Close();

	/** 门状态改变时广播 */
	UPROPERTY(BlueprintAssignable, Category = "Door")
	FOnDoorStateChanged OnDoorStateChanged;

	/** 开门动画时长 (秒) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door")
	float OpenDuration = 0.5f;

protected:
	virtual void BeginPlay() override;

	/** 开门动画 (简单插值, 门升起或淡出) */
	void PlayOpenAnimation();

	/** 动画 tick */
	virtual void Tick(float DeltaTime) override;

	// ─── 组件 ────────────────────────────────────────────────────────
	/** 碰撞盒 (阻挡玩家) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UBoxComponent* CollisionBox;

	/** 可视化网格 (门的样子) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UStaticMeshComponent* DoorMesh;

private:
	// 动画状态
	bool bIsAnimating = false;
	float AnimationProgress = 0.f;
	FVector StartLocation;
	FVector TargetLocation;

	/** 门升起的高度 (开门时) */
	UPROPERTY(EditDefaultsOnly, Category = "Door")
	float RaiseHeight = 300.f;
};
