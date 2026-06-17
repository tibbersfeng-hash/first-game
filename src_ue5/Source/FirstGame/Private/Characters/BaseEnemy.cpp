// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Characters/BaseEnemy.h"
#include "DataAssets/CharacterDataAsset.h"
#include "Combat/HitBoxComponent.h"
#include "Combat/HurtBoxComponent.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystemComponent->SetIsReplicated(false);

	EnemyHitBox = CreateDefaultSubobject<UHitBoxComponent>(TEXT("EnemyHitBox"));
	EnemyHitBox->SetupAttachment(RootComponent);
	EnemyHitBox->SetHiddenInGame(true);

	EnemyHurtBox = CreateDefaultSubobject<UHurtBoxComponent>(TEXT("EnemyHurtBox"));
	EnemyHurtBox->SetupAttachment(RootComponent);

	CurrentHealth = 50.f;
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
	}

	SetState("Idle");
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == "Dead") return;

	// Simple aggro check based on distance to player
	if (!bIsAggro)
	{
		// Would check player distance here in full implementation
	}
}

void ABaseEnemy::InitializeEnemy(UCharacterDataAsset* InDataAsset)
{
	EnemyData = InDataAsset;
	if (!EnemyData) return;

	CurrentHealth = EnemyData->MaxHealth;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = EnemyData->MoveSpeed * 0.7f; // Enemies slightly slower
	}

	UE_LOG(LogTemp, Log, TEXT("Enemy initialized: %s (HP:%.0f)"), *EnemyData->CharacterId.ToString(), CurrentHealth);
}

void ABaseEnemy::ReceiveHitDamage(float Amount, AActor* DamageCauser)
{
	if (CurrentState == "Dead") return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.f, EnemyData ? EnemyData->MaxHealth : 50.f);

	UE_LOG(LogTemp, Log, TEXT("Enemy took %.0f damage. HP: %.0f"), Amount, CurrentHealth);

	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnHitLanded.Broadcast(DamageCauser, this, Amount, GetActorLocation());

		if (CurrentHealth <= 0.f)
		{
			Die();
		}
		else
		{
			SetState("HitStun");
			bIsAggro = true; // Always aggro after being hit

			FTimerHandle Handle;
			GetWorldTimerManager().SetTimer(Handle, [this]()
			{
				SetState("Idle");
			}, EnemyData ? EnemyData->HitStunDuration : 0.3f, false);
		}
	}
}

void ABaseEnemy::PerformAttack()
{
	if (CurrentState != "Idle" || !bIsAggro) return;

	float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown) return;

	LastAttackTime = Now;
	SetState("Attacking");

	float Damage = EnemyData && EnemyData->LightAttacks.Num() > 0
		? EnemyData->LightAttacks[0].Damage : 10.f;

	EnemyHitBox->ActivateHitBox(Damage, this);

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, [this]()
	{
		EnemyHitBox->DeactivateHitBox();
		SetState("Idle");
	}, 0.5f, false);
}

void ABaseEnemy::Die()
{
	SetState("Dead");
	bIsAggro = false;

	UE_LOG(LogTemp, Log, TEXT("Enemy died: %s"), *GetName());

	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnEnemyDied.Broadcast(this);
	}

	// Play death animation, then destroy
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, [this]()
	{
		Destroy();
	}, 1.0f, false);
}

void ABaseEnemy::SetState(FName NewState)
{
	if (CurrentState != NewState)
	{
		FName OldState = CurrentState;
		CurrentState = NewState;
		OnStateChanged.Broadcast(this, NewState);
	}
}
