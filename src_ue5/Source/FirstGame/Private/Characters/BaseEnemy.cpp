// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Characters/BaseEnemy.h"
#include "DataAssets/CharacterDataAsset.h"
#include "DataAssets/CharacterDataFactory.h"
#include "Combat/HitBoxComponent.h"
#include "Combat/HurtBoxComponent.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Dungeon/LootItem.h"

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

	// 如果没有外部数据, 根据 EnemyType 创建对应数据
	if (!EnemyData)
	{
		UCharacterDataAsset* Data = nullptr;
		FString TypeName;

		switch (EnemyType)
		{
		case EEnemyType::CandyZombie:
			Data = UCharacterDataFactory::CreateDefaultEnemyData(this);
			TypeName = "CandyZombie";
			break;
		case EEnemyType::Gingerbread:
			Data = UCharacterDataFactory::CreateGingerbreadData(this);
			TypeName = "Gingerbread";
			break;
		case EEnemyType::ShadowNinja:
			Data = UCharacterDataFactory::CreateShadowNinjaData(this);
			TypeName = "ShadowNinja";
			break;
		case EEnemyType::ArmoredGum:
			Data = UCharacterDataFactory::CreateArmoredGumData(this);
			TypeName = "ArmoredGum";
			break;
		}

		if (Data)
		{
			InitializeEnemy(Data);
			UE_LOG(LogTemp, Log, TEXT("BaseEnemy: 使用 %s 数据"), *TypeName);
		}
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

	// 掉落战利品 (按 DropChance 概率)
	if (FMath::FRand() <= DropChance)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// 在敌人位置稍微偏上生成 (避免和地板穿插)
			FVector LootLocation = GetActorLocation() + FVector(0.f, 0.f, 50.f);

			ALootItem* Loot = World->SpawnActor<ALootItem>(
				ALootItem::StaticClass(),
				LootLocation,
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (Loot)
			{
				// 随机选择掉落类型 (50% 生命, 50% 能量)
				ELootType LootType = (FMath::FRand() < 0.5f)
					? ELootType::Health
					: ELootType::Energy;
				Loot->SetupLoot(LootType, LootValue);

				UE_LOG(LogTemp, Log,
					TEXT("Enemy 掉落战利品: Type=%d, Value=%.0f"),
					(int)LootType, LootValue);
			}
		}
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

// ─── ILockableTarget 实现 ───────────────────────────────────────────

float ABaseEnemy::GetHPRatio_Implementation() const
{
	const float MaxHP = EnemyData ? EnemyData->MaxHealth : 50.f;
	return MaxHP > 0.f ? FMath::Clamp(CurrentHealth / MaxHP, 0.f, 1.f) : 0.f;
}

bool ABaseEnemy::IsLockable_Implementation() const
{
	// 死亡状态不可锁定
	return CurrentState != "Dead" && CurrentHealth > 0.f;
}

float ABaseEnemy::GetLockPriority_Implementation() const
{
	// Boss 类型优先级更高 (ArmoredGum 是 Boss 类型)
	switch (EnemyType)
	{
	case EEnemyType::ArmoredGum:    return 1.5f;  // Boss
	case EEnemyType::ShadowNinja:   return 1.2f;  // Elite
	case EEnemyType::Gingerbread:   return 1.0f;  // Normal
	case EEnemyType::CandyZombie:   return 0.8f;  // Minion
	default:                         return 1.0f;
	}
}
