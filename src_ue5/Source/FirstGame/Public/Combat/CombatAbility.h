// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Combat Ability — GAS-based attack ability base class

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CombatAbility.generated.h"

class UHitBoxComponent;

/**
 * Base class for all combat abilities (light attack, heavy attack, special).
 * Handles startup/active/recovery frame timing and HitBox activation.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTGAME_API UCombatAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCombatAbility();

	// ─── Attack Parameters ───────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Frames")
	float StartupFrames = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Frames")
	float ActiveFrames = 4.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Frames")
	float RecoveryFrames = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Frames")
	float HitStopFrames = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Effects")
	FVector2D KnockbackDirection = FVector2D(100.f, -50.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Effects")
	bool bLaunchesEnemy = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Effects")
	float LaunchForce = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Effects")
	float EnergyCost = 5.f;

	// ─── Animation ──────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* AttackMontage;

	// ─── GAS Interface ──────────────────────────────────────────────

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Called during the active window — activate HitBox */
	UFUNCTION(BlueprintImplementableEvent, Category = "Attack")
	void OnActiveWindow();

	/** Frame to seconds conversion (60fps基准) */
	UFUNCTION(BlueprintPure, Category = "Attack")
	float FramesToSeconds(float Frames) const { return Frames / 60.f; }

private:
	void ActivateHitBox();
	void DeactivateHitBox();
	void ApplyHitStop();

	UPROPERTY()
	FTimerHandle StartupTimerHandle;

	UPROPERTY()
	FTimerHandle ActiveTimerHandle;

	UPROPERTY()
	FTimerHandle RecoveryTimerHandle;
};
