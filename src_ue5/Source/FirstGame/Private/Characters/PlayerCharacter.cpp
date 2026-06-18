// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Player Character — 3D third-person action fighter

#include "FirstGame.h"
#include "Characters/PlayerCharacter.h"
#include "Combat/HitBoxComponent.h"
#include "Combat/HurtBoxComponent.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "Subsystems/CombatDataSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraController.h"
#include "LockOn/LockOnComponent.h"
#include "Combat/ComboManager.h"
#include "Stats/CharacterStatsComponent.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystemComponent->SetIsReplicated(false);

	// Create HitBox (attack detection)
	ActiveHitBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("ActiveHitBox"));
	ActiveHitBox->SetupAttachment(RootComponent);
	ActiveHitBox->SetHiddenInGame(true);

	// Create HurtBox (damage receiving)
	PlayerHurtBox = CreateDefaultSubobject<UHurtBoxComponent>(TEXT("PlayerHurtBox"));
	PlayerHurtBox->SetupAttachment(RootComponent);

	// ─── Camera (ADR-008) ────────────────────────────────────────────
	// SpringArm: 相机伸缩杆 (负责距离/碰撞/平滑跟随)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.f;           // 自由模式默认距离
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 80.f);  // 相机高度 (稍高于角色)
	CameraBoom->bUsePawnControlRotation = true;    // 跟随 Controller 旋转
	CameraBoom->bDoCollisionTest = true;           // 启用碰撞避免穿墙
	CameraBoom->bEnableCameraLag = true;           // 平滑跟随
	CameraBoom->CameraLagSpeed = 10.f;
	CameraBoom->bEnableCameraRotationLag = true;   // 旋转平滑
	CameraBoom->CameraRotationLagSpeed = 5.f;

	// Camera: 第三人称视角
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->SetFieldOfView(65.f);            // 自由模式默认 FOV

	// CameraController: 管理 4 种相机模式
	CameraController = CreateDefaultSubobject<UCameraController>(TEXT("CameraController"));

	// LockOnComponent: 目标锁定系统 (ADR-009)
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));

	// ComboManager: 连招管理器 (ADR-007)
	ComboManager = CreateDefaultSubobject<UComboManager>(TEXT("ComboManager"));

	// CharacterStatsComponent: 运行时数值管理
	StatsComponent = CreateDefaultSubobject<UCharacterStatsComponent>(TEXT("StatsComponent"));

	// Default stats (will be overridden by DataAsset)
	CurrentHealth = 100.f;
	CurrentEnergy = 100.f;
	CurrentComboCount = 0;
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// ─── 3D Movement: 移除 2D 平面约束 ───────────────────────────
	// 3D 第三人称: 全向移动，不再锁定到 XZ 平面
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bConstrainToPlane = false;
		MoveComp->bUseControllerDesiredRotation = true;  // 朝向控制器旋转方向
		MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f); // 转身速度
	}

	SetState("Idle");
	UE_LOG(LogTemp, Log, TEXT("PlayerCharacter initialized in 3D mode"));

	// 初始化 CameraController (传入 SpringArm 和 Camera)
	if (CameraController && CameraBoom && FollowCamera)
	{
		CameraController->Initialize(CameraBoom, FollowCamera);
		UE_LOG(LogTemp, Log, TEXT("PlayerCharacter: CameraController 已初始化"));
	}

	// 初始化 LockOn: 绑定目标变化回调
	if (LockOnComponent)
	{
		LockOnComponent->OnLockChanged.AddDynamic(this, &APlayerCharacter::OnLockTargetChanged);
		UE_LOG(LogTemp, Log, TEXT("PlayerCharacter: LockOnComponent 已初始化"));
	}

	// ComboManager 已在构造函数中创建，无需额外初始化
	if (ComboManager)
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerCharacter: ComboManager 已初始化 (窗口:%.1fs)"),
			ComboManager->ComboWindowTime);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Hit stop timer
	if (bIsInHitStop)
	{
		HitStopTimer -= DeltaTime;
		if (HitStopTimer <= 0.f)
		{
			bIsInHitStop = false;
			UE_LOG(LogTemp, Log, TEXT("HitStop ended"));
		}
		return; // Skip all logic during hit stop
	}

	// Combo reset timer — 由 ComboManager 的 TickComponent 处理
	// 同步 HUD 显示用的连击计数
	CurrentComboCount = ComboManager ? ComboManager->GetCurrentCount() : 0;

	// Energy regen — 由 StatsComponent::TickComponent 处理
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 3D Input bindings (Enhanced Input System in production)
	// WASD: Move2D = 前后, Strafe = 左右
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::Move2D);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::Strafe);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump2D);
	PlayerInputComponent->BindAction("LightAttack", IE_Pressed, this, &APlayerCharacter::PerformLightAttack);
	PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &APlayerCharacter::PerformHeavyAttack);
	PlayerInputComponent->BindAction("Special", IE_Pressed, this, &APlayerCharacter::PerformSpecialMove);
	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &APlayerCharacter::PerformDodge);
	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &APlayerCharacter::LockOnTarget);
	PlayerInputComponent->BindAction("SwitchLockOn", IE_Pressed, this, &APlayerCharacter::SwitchLockTargetNext);
}

void APlayerCharacter::InitializeCharacter(UCharacterDataAsset* InDataAsset)
{
	CharacterData = InDataAsset;
	if (!CharacterData) return;

	CurrentCharacterId = CharacterData->CharacterId;

	// 通过 StatsComponent 加载数值
	if (StatsComponent)
	{
		StatsComponent->LoadFromDataAsset(CharacterData);
		CurrentHealth = StatsComponent->GetCurrentHealth();
		CurrentEnergy = StatsComponent->GetCurrentEnergy();
	}
	else
	{
		// Fallback: 直接使用 DataAsset 数值
		CurrentHealth = CharacterData->MaxHealth;
		CurrentEnergy = CharacterData->MaxEnergy;
	}

	// Apply movement settings
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CharacterData->MoveSpeed;
		MoveComp->GravityScale = CharacterData->Gravity / 980.f;
		MoveComp->JumpZVelocity = CharacterData->JumpForce;
	}

	UE_LOG(LogTemp, Log, TEXT("Character initialized: %s (HP:%.0f, Speed:%.0f)"),
		*CharacterData->CharacterId.ToString(), CurrentHealth, CharacterData->MoveSpeed);
}

// ─── Actions ─────────────────────────────────────────────────────────

void APlayerCharacter::PerformLightAttack()
{
	if (!CanAct()) return;

	if (!CharacterData || CharacterData->LightAttacks.Num() == 0) return;

	int32 Index = FMath::Clamp(0, 0, CharacterData->LightAttacks.Num() - 1);
	const FAttackMoveData& Move = CharacterData->LightAttacks[Index];

	if (!ConsumeEnergy(CharacterData->EnergyCostPerAttack)) return;

	SetState("Attacking");

	// 通过 ComboManager 注册命中
	if (ComboManager)
	{
		ComboManager->RegisterHit("LightAttack");
		CurrentComboCount = ComboManager->GetCurrentCount();
	}

	UE_LOG(LogTemp, Log, TEXT("Light Attack %d: Damage=%.0f, Startup=%.1f"), Index + 1, Move.Damage, Move.StartupFrames);

	// Activate HitBox after startup frames
	float StartupTime = Move.StartupFrames / 60.f; // Convert frames to seconds
	GetWorldTimerManager().SetTimerForNextTick([this, Move]()
	{
		ActiveHitBox->ActivateHitBox(Move.Damage, this);
	});

	// Deactivate after active frames
	float TotalTime = (Move.StartupFrames + Move.ActiveFrames) / 60.f;
	FTimerHandle DeactivateHandle;
	GetWorldTimerManager().SetTimer(DeactivateHandle, [this]()
	{
		ActiveHitBox->DeactivateHitBox();
		SetState("Idle");
	}, TotalTime, false);
}

void APlayerCharacter::PerformHeavyAttack()
{
	if (!CanAct()) return;
	if (!CharacterData) return;

	const FAttackMoveData& Move = CharacterData->HeavyAttack;
	if (!ConsumeEnergy(CharacterData->EnergyCostPerAttack * 2.f)) return;

	SetState("Attacking");

	// 重攻击打断连招
	if (ComboManager)
	{
		ComboManager->ResetCombo();
		CurrentComboCount = 0;
	}

	UE_LOG(LogTemp, Log, TEXT("Heavy Attack: Damage=%.0f"), Move.Damage);

	ActiveHitBox->ActivateHitBox(Move.Damage, this);

	float TotalTime = (Move.StartupFrames + Move.ActiveFrames + Move.RecoveryFrames) / 60.f;
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, [this]()
	{
		ActiveHitBox->DeactivateHitBox();
		SetState("Idle");
	}, TotalTime, false);
}

void APlayerCharacter::PerformSpecialMove()
{
	if (!CanAct()) return;
	if (!CharacterData) return;

	const FAttackMoveData& Move = CharacterData->SpecialMove;
	float Cost = CharacterData->EnergyCostPerAttack * 5.f;
	if (!ConsumeEnergy(Cost)) return;

	SetState("SpecialAttacking");

	// 必杀技打断连招
	if (ComboManager)
	{
		ComboManager->ResetCombo();
		CurrentComboCount = 0;
	}

	UE_LOG(LogTemp, Log, TEXT("SPECIAL MOVE: Damage=%.0f!"), Move.Damage);

	// Special move: longer animation, more damage
	ActiveHitBox->ActivateHitBox(Move.Damage, this);
	ActiveHitBox->SetKnockback(Move.Knockback);
	ActiveHitBox->SetLaunch(Move.LaunchForce);

	float TotalTime = (Move.StartupFrames + Move.ActiveFrames + Move.RecoveryFrames) / 60.f;
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, [this]()
	{
		ActiveHitBox->DeactivateHitBox();
		SetState("Idle");
	}, TotalTime, false);
}

void APlayerCharacter::PerformDodge()
{
	if (!CanAct()) return;

	// ── 3D Dodge: 方向闪避 ────────────────────────────────────────
	// 基于最后移动方向 or 相机方向
	APlayerController* PC = Cast<APlayerController>(GetController());
	FVector DodgeDir = GetActorForwardVector();

	if (PC)
	{
		FRotator CameraRot = PC->GetControlRotation();
		FRotator YawRotation(0.f, CameraRot.Yaw, 0.f);
		DodgeDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	}

	// 锁定模式下: 闪避方向相对锁定目标
	if (LockOnComponent && LockOnComponent->IsLockedOn())
	{
		AActor* Target = LockOnComponent->GetLockedTarget();
		if (Target)
		{
			FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			DodgeDir = FVector(ToTarget.X, ToTarget.Y, 0.f).GetSafeNormal();
		}
	}

	// 体力检查
	if (StatsComponent)
	{
		const float StaminaCost = 15.f;
		if (StatsComponent->GetCurrentStamina() < StaminaCost)
		{
			UE_LOG(LogTemp, Log, TEXT("Dodge failed: not enough stamina"));
			return;
		}
		StatsComponent->SetCurrentStamina(StatsComponent->GetCurrentStamina() - StaminaCost);
	}

	SetState("Dodging");

	// 位移: 3m 闪避距离
	const float DodgeDistance = 300.f; // cm
	AddMovementInput(DodgeDir, 1.f);

	// Dodge 持续时间: 13f @ 60fps = 216ms
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, [this]()
	{
		SetState("Idle");
	}, 0.216f, false);

	UE_LOG(LogTemp, Log, TEXT("Dodge! Dir: %s"), *DodgeDir.ToString());
}

void APlayerCharacter::JumpAction()
{
	if (!CanAct()) return;
	Jump2D();
}

// ─── Damage ──────────────────────────────────────────────────────────

void APlayerCharacter::ReceiveHitDamage(float Amount, AActor* DamageCauser)
{
	float OldHP = StatsComponent->GetCurrentHealth();
	StatsComponent->SetCurrentHealth(OldHP - Amount);
	CurrentHealth = StatsComponent->GetCurrentHealth();

	UE_LOG(LogTemp, Log, TEXT("Player took %.0f damage. HP: %.0f"), Amount, CurrentHealth);

	// Bug #2 fix: 死亡检测独立于 SignalBus, 防止空指针时玩家以 0 HP 继续行动
	if (CurrentHealth <= 0.f)
	{
		SetState("Dead");

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetInputMode(FInputModeUIOnly());
			PC->bShowMouseCursor = true;
		}
	}

	// 广播信号
	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnPlayerHealthChanged.Broadcast(this, CurrentHealth);

		if (CurrentHealth <= 0.f)
		{
			SignalBus->OnPlayerDied.Broadcast(this);
		}
		else
		{
			SetState("HitStun");
			ApplyHitStop(0.1f);
		}
	}
}

void APlayerCharacter::Heal(float Amount)
{
	StatsComponent->Heal(Amount);
	CurrentHealth = StatsComponent->GetCurrentHealth();
}

// ─── Hit Stop ────────────────────────────────────────────────────────

void APlayerCharacter::ApplyHitStop(float Duration)
{
	bIsInHitStop = true;
	HitStopTimer = Duration;
	UE_LOG(LogTemp, Log, TEXT("HitStop: %.3f seconds"), Duration);
}

// ── Energy ──────────────────────────────────────────────────────────

void APlayerCharacter::AddEnergy(float Amount)
{
	StatsComponent->SetCurrentEnergy(StatsComponent->GetCurrentEnergy() + Amount);
	CurrentEnergy = StatsComponent->GetCurrentEnergy();

	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnPlayerEnergyChanged.Broadcast(this, CurrentEnergy);
	}
}

bool APlayerCharacter::ConsumeEnergy(float Amount)
{
	bool bResult = StatsComponent->ConsumeEnergy(Amount);
	CurrentEnergy = StatsComponent->GetCurrentEnergy();

	if (bResult)
	{
		USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
		if (SignalBus)
		{
			SignalBus->OnPlayerEnergyChanged.Broadcast(this, CurrentEnergy);
		}
	}
	return bResult;
}

bool APlayerCharacter::HasEnoughEnergy(float Amount) const
{
	return StatsComponent->GetCurrentEnergy() >= Amount;
}

// ─── State Machine ───────────────────────────────────────────────────

bool APlayerCharacter::CanAct() const
{
	if (bIsInHitStop) return false;
	if (CurrentState == "Dead" || CurrentState == "HitStun" || CurrentState == "Attacking" || CurrentState == "SpecialAttacking")
		return false;
	return true;
}

void APlayerCharacter::SetState(FName NewState)
{
	if (CurrentState != NewState)
	{
		FName OldState = CurrentState;
		CurrentState = NewState;
		OnStateChanged.Broadcast(this, NewState);
		UE_LOG(LogTemp, Log, TEXT("State: %s -> %s"), *OldState.ToString(), *NewState.ToString());
	}
}

// ─── 3D Movement ───────────────────────────────────────────────────

void APlayerCharacter::Move2D(float Value)
{
	if (!CanAct() && CurrentState != "Dodging") return;

	// 3D: 基于相机方向的前后移动
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FRotator CameraRot = PC->GetControlRotation();
	FRotator YawRotation(0.f, CameraRot.Yaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// 锁定模式下: 前后 = 接近/远离目标
	if (LockOnComponent && LockOnComponent->IsLockedOn())
	{
		AActor* Target = LockOnComponent->GetLockedTarget();
		if (Target)
		{
			FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			FVector Forward = FVector(ToTarget.X, ToTarget.Y, 0.f).GetSafeNormal();
			AddMovementInput(Forward, Value);
		}
	}
	else
	{
		AddMovementInput(ForwardDir, Value);
	}
}

/** 3D 横向移动（环绕锁定目标） */
void APlayerCharacter::Strafe(float Value)
{
	if (!CanAct() && CurrentState != "Dodging") return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// 锁定模式下: 左右 = 环绕目标
	if (LockOnComponent && LockOnComponent->IsLockedOn())
	{
		AActor* Target = LockOnComponent->GetLockedTarget();
		if (Target)
		{
			FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			FVector Forward = FVector(ToTarget.X, ToTarget.Y, 0.f).GetSafeNormal();
			FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
			AddMovementInput(Right, Value);
		}
	}
	else
	{
		FRotator CameraRot = PC->GetControlRotation();
		FRotator YawRotation(0.f, CameraRot.Yaw, 0.f);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDir, Value);
	}
}

void APlayerCharacter::Jump2D()
{
	if (!CanAct()) return;
	Jump();
}

// ─── Lock-On (ADR-009) ───────────────────────────────────────────────

void APlayerCharacter::LockOnTarget()
{
	if (!LockOnComponent) { return; }

	// 已有锁定时 → 释放；无锁定时 → 锁定最近
	if (LockOnComponent->IsLockedOn())
	{
		ReleaseLockOn();
	}
	else
	{
		LockOnComponent->LockOnNearest();
	}
}

void APlayerCharacter::SwitchLockTarget(bool bNext /*= true*/)
{
	if (!LockOnComponent) { return; }

	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->SwitchTarget(bNext);
	}
	else
	{
		// 未锁定时按切换键 = 锁定最近
		LockOnComponent->LockOnNearest();
	}
}

void APlayerCharacter::SwitchLockTargetNext()
{
	SwitchLockTarget(true);
}

void APlayerCharacter::ReleaseLockOn()
{
	if (!LockOnComponent) { return; }
	LockOnComponent->ReleaseLock();
}

void APlayerCharacter::OnLockTargetChanged(AActor* NewTarget)
{
	// 通知相机系统切换模式
	if (CameraController)
	{
		if (NewTarget)
		{
			CameraController->SetMode(ECameraMode::Locked);
			UE_LOG(LogTemp, Log, TEXT("LockOn: Camera → Locked mode (target: %s)"),
				*NewTarget->GetActorLabel());
		}
		else
		{
			CameraController->SetMode(ECameraMode::Free);
			UE_LOG(LogTemp, Log, TEXT("LockOn: Camera → Free mode"));
		}
	}

	// 通知 HUD 锁定状态
	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnLockOnTargetChanged.Broadcast(this, NewTarget);
	}
}
