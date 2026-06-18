// Copyright 2026 格斗萌主 Team. All Rights Reserved.

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

	// Default stats
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

	// Apply 2D plane constraint (lock to XZ plane for side-scrolling)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f)); // Lock Y axis
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
	}

	SetState("Idle");
	UE_LOG(LogTemp, Log, TEXT("PlayerCharacter initialized in 2.5D mode"));

	// 初始化 CameraController (传入 SpringArm 和 Camera)
	if (CameraController && CameraBoom && FollowCamera)
	{
		CameraController->Initialize(CameraBoom, FollowCamera);
		UE_LOG(LogTemp, Log, TEXT("PlayerCharacter: CameraController 已初始化"));
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

	// Combo reset timer
	if (CurrentComboCount > 0)
	{
		ComboTimer -= DeltaTime;
		if (ComboTimer <= 0.f)
		{
			ResetCombo();
		}
	}

	// Energy regen
	if (CharacterData && CurrentEnergy < CharacterData->MaxEnergy)
	{
		AddEnergy(CharacterData->EnergyRegenRate * DeltaTime);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input bindings will be configured via Enhanced Input System in production
	// Placeholder bindings for testing
	PlayerInputComponent->BindAxis("Move2D", this, &APlayerCharacter::Move2D);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump2D);
	PlayerInputComponent->BindAction("LightAttack", IE_Pressed, this, &APlayerCharacter::PerformLightAttack);
	PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &APlayerCharacter::PerformHeavyAttack);
	PlayerInputComponent->BindAction("Special", IE_Pressed, this, &APlayerCharacter::PerformSpecialMove);
	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &APlayerCharacter::PerformDodge);
}

void APlayerCharacter::InitializeCharacter(UCharacterDataAsset* InDataAsset)
{
	CharacterData = InDataAsset;
	if (!CharacterData) return;

	CurrentHealth = CharacterData->MaxHealth;
	CurrentEnergy = CharacterData->MaxEnergy;
	CurrentCharacterId = CharacterData->CharacterId;

	// Apply movement settings
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CharacterData->MoveSpeed;
		MoveComp->GravityScale = CharacterData->Gravity / 980.f;
		MoveComp->JumpZVelocity = CharacterData->JumpForce;
		// MaxJumps handled via custom CanJump() override in production
	}

	UE_LOG(LogTemp, Log, TEXT("Character initialized: %s (HP:%.0f, SPD:%.0f)"),
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
	UpdateCombo();

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
	ResetCombo();

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
	ResetCombo();

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

	SetState("Dodging");

	// Brief invincibility + dash
	UE_LOG(LogTemp, Log, TEXT("Dodge!"));

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, [this]()
	{
		SetState("Idle");
	}, 0.3f, false);
}

void APlayerCharacter::JumpAction()
{
	if (!CanAct()) return;
	Jump2D();
}

// ─── Damage ──────────────────────────────────────────────────────────

void APlayerCharacter::ReceiveHitDamage(float Amount, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.f, CharacterData ? CharacterData->MaxHealth : 100.f);

	UE_LOG(LogTemp, Log, TEXT("Player took %.0f damage. HP: %.0f"), Amount, CurrentHealth);

	// Broadcast signal
	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnPlayerHealthChanged.Broadcast(this, CurrentHealth);

		if (CurrentHealth <= 0.f)
		{
			SignalBus->OnPlayerDied.Broadcast(this);
			SetState("Dead");
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
	float MaxHP = CharacterData ? CharacterData->MaxHealth : 100.f;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHP);
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
	float MaxEnergy = CharacterData ? CharacterData->MaxEnergy : 100.f;
	CurrentEnergy = FMath::Clamp(CurrentEnergy + Amount, 0.f, MaxEnergy);

	// 通知 HUD 能量变化
	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnPlayerEnergyChanged.Broadcast(this, CurrentEnergy);
	}
}

bool APlayerCharacter::ConsumeEnergy(float Amount)
{
	if (CurrentEnergy < Amount)
	{
		UE_LOG(LogTemp, Log, TEXT("Not enough energy: need %.0f, have %.0f"), Amount, CurrentEnergy);
		return false;
	}
	CurrentEnergy -= Amount;

	// 通知 HUD 能量变化
	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnPlayerEnergyChanged.Broadcast(this, CurrentEnergy);
	}
	return true;
}

bool APlayerCharacter::HasEnoughEnergy(float Amount) const
{
	return CurrentEnergy >= Amount;
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

void APlayerCharacter::ResetCombo()
{
	if (CurrentComboCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Combo reset (was %d)"), CurrentComboCount);
		CurrentComboCount = 0;
		ComboTimer = 0.f;
	}
}

void APlayerCharacter::UpdateCombo()
{
	CurrentComboCount++;
	ComboTimer = 1.5f; // Reset timeout
	UE_LOG(LogTemp, Log, TEXT("Combo: %d"), CurrentComboCount);

	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnComboUpdated.Broadcast(this, CurrentComboCount);
	}
}

// ─── Movement ────────────────────────────────────────────────────────

void APlayerCharacter::Move2D(float Value)
{
	if (!CanAct() && CurrentState != "Dodging") return;

	const FVector Direction = FVector(Value, 0.f, 0.f);
	AddMovementInput(Direction, Value);
}

void APlayerCharacter::Jump2D()
{
	if (!CanAct()) return;
	Jump();
}
