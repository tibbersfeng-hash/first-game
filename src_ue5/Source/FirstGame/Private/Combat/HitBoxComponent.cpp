// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Combat/HitBoxComponent.h"
#include "FirstGame.h"
#include "Combat/HurtBoxComponent.h"
#include "Combat/HitCameraShake.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "Subsystems/SignalBusTypes.h"
#include "Subsystems/AudioManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Engine/DamageEvents.h"

UHitBoxComponent::UHitBoxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionEnabled(ECollisionEnabled::NoCollision); // Start disabled
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECR_Overlap); // HurtBox channel

	// SetBoxExtent deferred to BeginPlay — calling it in constructor triggers
	// UBoxComponent::UpdateBodySetup() which uses NewObject inside UObject constructor
}

void UHitBoxComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set extent now that we're outside UObject construction
	SetBoxExtent(FVector(40.f, 10.f, 60.f));

	OnComponentBeginOverlap.AddDynamic(this, &UHitBoxComponent::OnHitBeginOverlap);
}

void UHitBoxComponent::ActivateHitBox(float InDamage, AActor* InAttacker)
{
	CurrentDamage = InDamage;
	CurrentAttacker = InAttacker;
	bHitBoxEnabled = true;

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UE_LOG(LogTemp, Log, TEXT("HitBox activated: Damage=%.0f"), InDamage);
}

void UHitBoxComponent::DeactivateHitBox()
{
	bHitBoxEnabled = false;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UHitBoxComponent::OnHitBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bHitBoxEnabled || !CurrentAttacker) return;
	if (OtherActor == CurrentAttacker) return; // Don't self-hit

	// Check if the other actor has a HurtBox
	UHurtBoxComponent* HurtBox = Cast<UHurtBoxComponent>(OtherComp);
	if (!HurtBox) return;

	// 检查无敌状态 (闪避/受击无敌帧)
	if (HurtBox->IsInvincible())
	{
		UE_LOG(LogTemp, Verbose, TEXT("HIT BLOCKED: Target is invincible"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HIT! Attacker=%s -> Target=%s, Damage=%.0f"),
		*GetNameSafe(CurrentAttacker), *GetNameSafe(OtherActor), CurrentDamage);

	// Broadcast hit event
	OnHit.Broadcast(CurrentAttacker, OtherActor, CurrentDamage);

	// Apply damage to target via TakeDamage
	if (OtherActor)
	{
		AController* Instigator = CurrentAttacker ? CurrentAttacker->GetInstigatorController() : nullptr;
		FDamageEvent DamageEvent;
		OtherActor->TakeDamage(CurrentDamage, DamageEvent, Instigator, CurrentAttacker);
	}

	// Trigger screen effects via SignalBus
	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnHitStopRequested.Broadcast(0.08f);
		SignalBus->OnShowDamageNumber.Broadcast(
			FDamageNumberData{SweepResult.Location, CurrentDamage, false, FLinearColor::White});
	}

	// Trigger camera shake on player's controller
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->ClientStartCameraShake(UHitCameraShake::StaticClass());
	}

	// Trigger hit sound
	UAudioManagerSubsystem* Audio = GetWorld()->GetGameInstance()->GetSubsystem<UAudioManagerSubsystem>();
	if (Audio)
	{
		Audio->PlayHitSound();
	}

	// Deactivate immediately (single-hit per activation)
	DeactivateHitBox();
}
