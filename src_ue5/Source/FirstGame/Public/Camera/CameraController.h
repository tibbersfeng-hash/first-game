// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Camera Controller — 管理第三人称相机的 4 种模式 (ADR-008)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraController.generated.h"

class USpringArmComponent;
class UCameraComponent;

/** 相机模式 (ADR-008) */
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	Free        UMETA(DisplayName = "Free"),        // 自由越肩
	Locked      UMETA(DisplayName = "Locked"),      // 锁定跟随
	Dodge       UMETA(DisplayName = "Dodge"),       // 极限闪避
	Ultimate    UMETA(DisplayName = "Ultimate")     // 必杀演出
};

/** 每种相机模式的参数 */
USTRUCT(BlueprintType)
struct FCameraModeParams
{
	GENERATED_BODY()

	/** SpringArm 长度 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float Distance = 250.f;

	/** 相机相对角色根节点的高度偏移 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float HeightOffset = 80.f;

	/** 视场角 (度) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float FOV = 65.f;

	/** 跟随平滑度 (越大越跟手, 越小越平滑) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float LagSpeed = 10.f;

	/** 旋转平滑度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float RotationSpeed = 5.f;

	/** 是否自动回正 (Free 模式用) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	bool bAutoCenter = false;
};

/**
 * 相机控制器 — 管理 PlayerCharacter 的 SpringArm + Camera
 *
 * 用法:
 *   在 PlayerCharacter 构造函数中 CreateDefaultSubobject<UCameraController>
 *   在 BeginPlay 中调用 Initialize() 传入 SpringArm 和 Camera
 *
 * 4 种模式切换时通过 FInterpTo 平滑过渡:
 *   - Distance (SpringArm 长度)
 *   - FOV (视场角)
 *   - HeightOffset (相机高度)
 */
UCLASS(ClassGroup = "(Custom)", meta = (BlueprintSpawnableComponent))
class FIRSTGAME_API UCameraController : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraController();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// ─── 初始化 ──────────────────────────────────────────────────────
	/** 绑定 SpringArm + Camera (在 PlayerCharacter::BeginPlay 调用) */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	// ─── 模式控制 ────────────────────────────────────────────────────
	/** 切换到新相机模式 (平滑过渡) */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetMode(ECameraMode NewMode);

	/** 强制立即切换到新相机模式 (无过渡) */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetModeImmediate(ECameraMode NewMode);

	/** 获取当前模式 */
	UFUNCTION(BlueprintPure, Category = "Camera")
	ECameraMode GetCurrentMode() const { return CurrentMode; }

	// ─── 输入通知 ────────────────────────────────────────────────────
	/** 玩家有相机输入时调用 (重置自动回正计时器) */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void NotifyCameraInput();

	// ─── 每种模式的参数 (蓝图可配置) ───────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Modes")
	FCameraModeParams FreeParams;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Modes")
	FCameraModeParams LockedParams;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Modes")
	FCameraModeParams DodgeParams;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Modes")
	FCameraModeParams UltimateParams;

	// ─── 过渡参数 ────────────────────────────────────────────────────
	/** 距离/高度插值速度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Blend")
	float DistanceBlendSpeed = 8.f;

	/** FOV 插值速度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Blend")
	float FOVBlendSpeed = 6.f;

	/** 自动回正延迟 (秒) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Blend")
	float AutoCenterDelay = 3.f;

	/** 自动回正速度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Blend")
	float AutoCenterSpeed = 1.5f;

protected:
	virtual void BeginPlay() override;

private:
	// ─── 内部更新 ────────────────────────────────────────────────────
	void UpdateCamera(float DeltaTime);
	void UpdateAutoCenter(float DeltaTime);
	FCameraModeParams GetModeParams(ECameraMode Mode) const;

	// ─── 绑定 ────────────────────────────────────────────────────────
	UPROPERTY()
	USpringArmComponent* SpringArm;

	UPROPERTY()
	UCameraComponent* Camera;

	// ─── 状态 ────────────────────────────────────────────────────────
	ECameraMode CurrentMode = ECameraMode::Free;

	// 当前插值中的实际参数
	float CurrentDistance = 250.f;
	float CurrentFOV = 65.f;
	float CurrentHeightOffset = 80.f;

	// 自动回正状态
	float TimeSinceLastInput = 0.f;

	bool bInitialized = false;
};
