// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Player Character — 2.5D side-scrolling fighter

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UCharacterDataAsset;
class UHitBoxComponent;
class UHurtBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UCameraController;
class ULockOnComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateChanged, class APlayerCharacter*, Player, FName, NewState);

UCLASS()
class FIRSTGAME_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	// ─── AbilitySystemInterface ──────────────────────────────────────
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ─── Setup ───────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Setup")
	void InitializeCharacter(UCharacterDataAsset* InDataAsset);

	// ─── Properties ──────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	UCharacterDataAsset* CharacterData;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	FName CurrentCharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 CurrentComboCount;

	// ─── States ──────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "State")
	FName GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "State")
	bool CanAct() const;

	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnPlayerStateChanged OnStateChanged;

	// ─── Actions ─────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Action")
	void PerformLightAttack();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PerformHeavyAttack();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PerformSpecialMove();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PerformDodge();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void JumpAction();

	// ─── Lock-On (ADR-009) ───────────────────────────────────────────
	/** 锁定最近目标 (Tab/中键) */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void LockOnTarget();

	/** 切换锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SwitchLockTarget(bool bNext = true);

	/** 释放锁定 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void ReleaseLockOn();

	// ─── Damage ──────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ReceiveHitDamage(float Amount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void Heal(float Amount);

	// ─── Hit Stop ────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyHitStop(float Duration);

	// ── Energy ───────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Energy")
	void AddEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Energy")
	bool ConsumeEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Energy")
	bool HasEnoughEnergy(float Amount) const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ─── 2D Movement ─────────────────────────────────────────────────
	void Move2D(float Value);
	void Jump2D();

	// ─── State Transitions ───────────────────────────────────────────
	void SetState(FName NewState);

	// ─── Combat Helpers ──────────────────────────────────────────────
	void ResetCombo();
	void UpdateCombo();

	// ─── Lock-On Helpers ─────────────────────────────────────────────
	UFUNCTION()
	void OnLockTargetChanged(AActor* NewTarget);

	/** 切换到下一个锁定目标 (输入绑定用无参版本) */
	void SwitchLockTargetNext();

	// ─── Components ──────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UHitBoxComponent* ActiveHitBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UHurtBoxComponent* PlayerHurtBox;

	// ─── Camera (ADR-008) ────────────────────────────────────────────
	/** SpringArm: 相机伸缩杆, 负责距离/碰撞/平滑跟随 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	/** Camera: 第三人称视角 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	/** CameraController: 管理 4 种相机模式切换 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraController* CameraController;

	/** LockOnComponent: 目标锁定系统 (ADR-009) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	ULockOnComponent* LockOnComponent;

public:
	/** 获取相机控制器 (便于外部切换模式) */
	UFUNCTION(BlueprintPure, Category = "Camera")
	UCameraController* GetCameraController() const { return CameraController; }

private:
	// ─── State ───────────────────────────────────────────────────────
	UPROPERTY()
	FName CurrentState = "Idle";

	float ComboTimer = 0.f;
	float HitStopTimer = 0.f;
	bool bIsInHitStop = false;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* JumpActionInput;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* LightAttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* HeavyAttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* SpecialAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* DodgeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* LockOnAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* SwitchLockOnAction;
};
