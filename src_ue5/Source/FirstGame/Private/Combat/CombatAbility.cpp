// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Combat/CombatAbility.h"
#include "Combat/HitBoxComponent.h"
#include "Characters/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "Subsystems/CombatDataSubsystem.h"
#include "TimerManager.h"

UCombatAbility::UCombatAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UCombatAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APlayerCharacter* Player = Cast<APlayerCharacter>(ActorInfo->AvatarActor);
	if (!Player)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	// Check energy
	if (!Player->ConsumeEnergy(EnergyCost))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	// Play montage if assigned
	if (AttackMontage)
	{
		Player->PlayAnimMontage(AttackMontage);
	}

	UE_LOG(LogTemp, Log, TEXT("CombatAbility activated: Damage=%.0f, Startup=%.1f frames"), Damage, StartupFrames);

	// Startup phase → wait for startup frames
	float StartupTime = FramesToSeconds(StartupFrames);
	GetWorld()->GetTimerManager().SetTimer(StartupTimerHandle, [this]()
	{
		// Active phase
		ActivateHitBox();

		float ActiveTime = FramesToSeconds(ActiveFrames);
		GetWorld()->GetTimerManager().SetTimer(ActiveTimerHandle, [this]()
		{
			DeactivateHitBox();

			// Recovery phase
			float RecoveryTime = FramesToSeconds(RecoveryFrames);
			GetWorld()->GetTimerManager().SetTimer(RecoveryTimerHandle, [this]()
			{
				Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
			}, RecoveryTime, false);

		}, ActiveTime, false);

	}, StartupTime, false);
}

void UCombatAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clear all timers
	GetWorld()->GetTimerManager().ClearTimer(StartupTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ActiveTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);

	DeactivateHitBox();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCombatAbility::ActivateHitBox()
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!Player) return;

	// Find HitBox component on the player
	UHitBoxComponent* HitBox = Player->FindComponentByClass<UHitBoxComponent>();
	if (HitBox)
	{
		HitBox->ActivateHitBox(Damage, Player);
		HitBox->SetKnockback(KnockbackDirection);
		HitBox->SetLaunch(bLaunchesEnemy ? LaunchForce : 0.f);
	}

	// Call blueprint event
	OnActiveWindow();

	// Apply hit stop
	ApplyHitStop();
}

void UCombatAbility::DeactivateHitBox()
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!Player) return;

	UHitBoxComponent* HitBox = Player->FindComponentByClass<UHitBoxComponent>();
	if (HitBox)
	{
		HitBox->DeactivateHitBox();
	}
}

void UCombatAbility::ApplyHitStop()
{
	float Duration = FramesToSeconds(HitStopFrames);
	APlayerCharacter* Player = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (Player)
	{
		Player->ApplyHitStop(Duration);
	}

	// Also broadcast signal for screen effects
	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnHitStopRequested.Broadcast(Duration);
	}
}
