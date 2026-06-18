// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Camera/CameraController.h"
#include "FirstGame.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

UCameraController::UCameraController()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 默认参数 (按 ADR-008 表格)
	FreeParams.Distance = 250.f;
	FreeParams.HeightOffset = 80.f;
	FreeParams.FOV = 65.f;
	FreeParams.LagSpeed = 10.f;
	FreeParams.RotationSpeed = 5.f;
	FreeParams.bAutoCenter = true;

	LockedParams.Distance = 200.f;
	LockedParams.HeightOffset = 100.f;
	LockedParams.FOV = 55.f;
	LockedParams.LagSpeed = 15.f;
	LockedParams.RotationSpeed = 8.f;
	LockedParams.bAutoCenter = false;

	DodgeParams.Distance = 150.f;
	DodgeParams.HeightOffset = 60.f;
	DodgeParams.FOV = 50.f;
	DodgeParams.LagSpeed = 12.f;
	DodgeParams.RotationSpeed = 6.f;
	DodgeParams.bAutoCenter = false;

	UltimateParams.Distance = 100.f;
	UltimateParams.HeightOffset = 50.f;
	UltimateParams.FOV = 45.f;
	UltimateParams.LagSpeed = 20.f;
	UltimateParams.RotationSpeed = 10.f;
	UltimateParams.bAutoCenter = false;
}

void UCameraController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("CameraController: BeginPlay"));
}

void UCameraController::Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	SpringArm = InSpringArm;
	Camera = InCamera;

	if (!SpringArm || !Camera)
	{
		UE_LOG(LogTemp, Error, TEXT("CameraController: SpringArm 或 Camera 为 null"));
		return;
	}

	// 初始参数 (从 Free 模式开始)
	CurrentDistance = FreeParams.Distance;
	CurrentFOV = FreeParams.FOV;
	CurrentHeightOffset = FreeParams.HeightOffset;

	// 应用初始值到 SpringArm
	SpringArm->TargetArmLength = CurrentDistance;
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, CurrentHeightOffset));
	SpringArm->bDoCollisionTest = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = FreeParams.LagSpeed;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = FreeParams.RotationSpeed;

	// 应用 FOV
	Camera->SetFieldOfView(CurrentFOV);

	CurrentMode = ECameraMode::Free;
	bInitialized = true;

	UE_LOG(LogTemp, Log,
		TEXT("CameraController: 初始化完成 (SpringArm=%.0f, FOV=%.0f, Height=%.0f)"),
		CurrentDistance, CurrentFOV, CurrentHeightOffset);
}

void UCameraController::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitialized) return;

	UpdateCamera(DeltaTime);
	UpdateAutoCenter(DeltaTime);
}

void UCameraController::SetMode(ECameraMode NewMode)
{
	if (NewMode == CurrentMode) return;

	UE_LOG(LogTemp, Log, TEXT("CameraController: 切换模式 %d → %d"),
		(int)CurrentMode, (int)NewMode);
	CurrentMode = NewMode;
	TimeSinceLastInput = 0.f;  // 切换模式时重置自动回正
}

void UCameraController::SetModeImmediate(ECameraMode NewMode)
{
	CurrentMode = NewMode;

	// 立即应用参数 (无过渡)
	const FCameraModeParams& Params = GetModeParams(NewMode);
	CurrentDistance = Params.Distance;
	CurrentFOV = Params.FOV;
	CurrentHeightOffset = Params.HeightOffset;

	if (SpringArm)
	{
		SpringArm->TargetArmLength = CurrentDistance;
		SpringArm->SetRelativeLocation(FVector(0.f, 0.f, CurrentHeightOffset));
		SpringArm->CameraLagSpeed = Params.LagSpeed;
		SpringArm->CameraRotationLagSpeed = Params.RotationSpeed;
	}
	if (Camera)
	{
		Camera->SetFieldOfView(CurrentFOV);
	}

	TimeSinceLastInput = 0.f;
}

void UCameraController::NotifyCameraInput()
{
	TimeSinceLastInput = 0.f;
}

void UCameraController::UpdateCamera(float DeltaTime)
{
	if (!SpringArm || !Camera) return;

	const FCameraModeParams& TargetParams = GetModeParams(CurrentMode);

	// 平滑过渡到目标参数
	CurrentDistance = FMath::FInterpTo(CurrentDistance, TargetParams.Distance,
	                                    DeltaTime, DistanceBlendSpeed);
	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetParams.FOV,
	                               DeltaTime, FOVBlendSpeed);
	CurrentHeightOffset = FMath::FInterpTo(CurrentHeightOffset, TargetParams.HeightOffset,
	                                        DeltaTime, DistanceBlendSpeed);

	// 应用到 SpringArm
	SpringArm->TargetArmLength = CurrentDistance;
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, CurrentHeightOffset));
	SpringArm->CameraLagSpeed = TargetParams.LagSpeed;
	SpringArm->CameraRotationLagSpeed = TargetParams.RotationSpeed;

	// 应用到 Camera
	Camera->SetFieldOfView(CurrentFOV);
}

void UCameraController::UpdateAutoCenter(float DeltaTime)
{
	if (!SpringArm) return;

	// 只在 Free 模式且开启了自动回正时生效
	if (CurrentMode != ECameraMode::Free || !FreeParams.bAutoCenter)
	{
		TimeSinceLastInput = 0.f;
		return;
	}

	TimeSinceLastInput += DeltaTime;

	if (TimeSinceLastInput > AutoCenterDelay)
	{
		// 缓慢回到角色背后 (Yaw = 0, Pitch = -10 轻微俯视)
		FRotator CurrentRot = SpringArm->GetRelativeRotation();
		FRotator TargetRot(0.f, 0.f, 0.f);  // 正后方
		TargetRot.Pitch = -10.f;              // 轻微俯视

		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, AutoCenterSpeed);
		SpringArm->SetRelativeRotation(NewRot);
	}
}

FCameraModeParams UCameraController::GetModeParams(ECameraMode Mode) const
{
	switch (Mode)
	{
	case ECameraMode::Free:      return FreeParams;
	case ECameraMode::Locked:    return LockedParams;
	case ECameraMode::Dodge:     return DodgeParams;
	case ECameraMode::Ultimate:  return UltimateParams;
	default:                     return FreeParams;
	}
}
