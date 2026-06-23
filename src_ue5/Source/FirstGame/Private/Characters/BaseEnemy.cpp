// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Characters/BaseEnemy.h"
#include "FirstGame.h"
#include "DataAssets/CharacterDataAsset.h"
#include "DataAssets/CharacterDataFactory.h"
#include "Combat/HitBoxComponent.h"
#include "Combat/HurtBoxComponent.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Dungeon/LootItem.h"
#include "Anim/MonsterAnimInstance.h"
#include "Utils/NPRMaterialUtils.h"

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

	// Auto-configure SkeletalMesh + AnimBP based on EnemyType
	ConfigureMonsterAssets();

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bConstrainToPlane = false;  // 3D 全向移动，不约束平面
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
	SetState("Idle");  // 确保初始状态正确（PostInitializeComponents 不会在 NewObject 时调用）

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = EnemyData->MoveSpeed * 0.7f; // Enemies slightly slower
		MoveComp->GravityScale = EnemyData->Gravity / 980.f;  // Fix flying bug
	}

	UE_LOG(LogTemp, Log, TEXT("Enemy initialized: %s (HP:%.0f, State:%s)"),
		*EnemyData->CharacterId.ToString(), CurrentHealth, *CurrentState.ToString());
}

void ABaseEnemy::ReceiveHitDamage(float Amount, AActor* DamageCauser)
{
	if (CurrentState == "Dead") return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.f, EnemyData ? EnemyData->MaxHealth : 50.f);

	UE_LOG(LogTemp, Log, TEXT("Enemy took %.0f damage. HP: %.0f"), Amount, CurrentHealth);

	// SignalBus 通知（可选）
	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnHitLanded.Broadcast(DamageCauser, this, Amount, GetActorLocation());
	}

	// 死亡/硬直逻辑 — 不依赖 SignalBus
	if (CurrentHealth <= 0.f)
	{
		Die();
	}
	else
	{
		SetState("HitStun");
		bIsAggro = true; // Always aggro after being hit

		// 测试环境中可能没有 World，跳过 Timer
		if (GetWorld())
		{
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
	if (!GetWorld()) return;  // 测试环境可能没有 World

	float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown) return;

	LastAttackTime = Now;
	SetState("Attacking");

	float Damage = EnemyData && EnemyData->LightAttacks.Num() > 0
		? EnemyData->LightAttacks[0].Damage : 10.f;

	EnemyHitBox->ActivateHitBox(Damage, this);

	if (GetWorld())
	{
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, [this]()
		{
			EnemyHitBox->DeactivateHitBox();
			SetState("Idle");
		}, 0.5f, false);
	}
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
	if (GetWorld())
	{
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, [this]()
		{
			Destroy();
		}, 1.0f, false);
	}
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

// ─── 动画控制 ──────────────────────────────────────────────────────

UMonsterAnimInstance* ABaseEnemy::GetMonsterAnimInstance() const
{
	return Cast<UMonsterAnimInstance>(GetMesh()->GetAnimInstance());
}

void ABaseEnemy::PlayHitAnimation()
{
	UMonsterAnimInstance* AnimInst = GetMonsterAnimInstance();
	if (AnimInst)
	{
		AnimInst->bIsHit = true;
		UE_LOG(LogTemp, Log, TEXT("BaseEnemy: PlayHitAnimation"));
	}
}

void ABaseEnemy::PlayDeathAnimation()
{
	UMonsterAnimInstance* AnimInst = GetMonsterAnimInstance();
	if (AnimInst)
	{
		AnimInst->bIsDead = true;
		UE_LOG(LogTemp, Log, TEXT("BaseEnemy: PlayDeathAnimation"));
	}
}

// ─── 自动加载怪物资产 ──────────────────────────────────────────────

void ABaseEnemy::ConfigureMonsterAssets()
{
	FString MeshPath;
	FString AnimBPPath;
	FLinearColor MonsterColor(1.0f, 1.0f, 1.0f, 1.0f);  // Default white

	switch (EnemyType)
	{
	case EEnemyType::CandyZombie:
		MeshPath = TEXT("/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie.SK_CandyZombie");
		AnimBPPath = TEXT("/Game/Monsters/CandyZombie/ABP/ABP_CandyZombie.ABP_CandyZombie");
		MonsterColor = FLinearColor(0.2f, 0.8f, 0.3f, 1.0f);  // Green
		break;
	case EEnemyType::Gingerbread:
		MeshPath = TEXT("/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread.SK_Gingerbread");
		AnimBPPath = TEXT("/Game/Monsters/Gingerbread/ABP/ABP_Gingerbread.ABP_Gingerbread");
		MonsterColor = FLinearColor(0.65f, 0.4f, 0.2f, 1.0f);  // Brown
		break;
	case EEnemyType::ShadowNinja:
		MeshPath = TEXT("/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja.SK_ShadowNinja");
		AnimBPPath = TEXT("/Game/Monsters/ShadowNinja/ABP/ABP_ShadowNinja.ABP_ShadowNinja");
		MonsterColor = FLinearColor(0.5f, 0.3f, 0.8f, 1.0f);  // Purple
		break;
	case EEnemyType::ArmoredGum:
		MeshPath = TEXT("/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum.SK_ArmoredGum");
		AnimBPPath = TEXT("/Game/Monsters/ArmoredGum/ABP/ABP_ArmoredGum.ABP_ArmoredGum");
		MonsterColor = FLinearColor(0.75f, 0.78f, 0.8f, 1.0f);  // Silver
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("BaseEnemy: Unknown EnemyType"));
		return;
	}

	// Load SkeletalMesh
	USkeletalMesh* LoadedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
	if (LoadedMesh)
	{
		GetMesh()->SetSkeletalMesh(LoadedMesh);
		UE_LOG(LogTemp, Log, TEXT("BaseEnemy: Set mesh %s for EnemyType %d"), *MeshPath, (int32)EnemyType);

		// Create Dynamic Material Instance with monster's signature color
		const TArray<FSkeletalMaterial>& MatArray = LoadedMesh->GetMaterials();
		UMaterialInterface* BaseMat = (MatArray.Num() > 0) ? MatArray[0].MaterialInterface : nullptr;
		if (!BaseMat)
		{
			// Fallback to DefaultMaterial if mesh has no material
			BaseMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
		}

		if (BaseMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(FName(TEXT("Base Color")), MonsterColor);
				DynMat->SetVectorParameterValue(FName(TEXT("BaseColor")), MonsterColor);  // Try alternate name
				GetMesh()->SetMaterial(0, DynMat);
				UE_LOG(LogTemp, Log, TEXT("BaseEnemy: Applied dynamic material (R=%.2f G=%.2f B=%.2f)"),
					MonsterColor.R, MonsterColor.G, MonsterColor.B);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseEnemy: Failed to load mesh %s"), *MeshPath);
	}

	// Load AnimBP class
	UClass* AnimBPClass = LoadObject<UClass>(nullptr, *AnimBPPath);
	if (AnimBPClass)
	{
		GetMesh()->SetAnimInstanceClass(AnimBPClass);
		UE_LOG(LogTemp, Log, TEXT("BaseEnemy: Set AnimBP %s"), *AnimBPPath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseEnemy: Failed to load AnimBP %s"), *AnimBPPath);
	}

	// 应用 NPR 材质
	UNPRMaterialUtils::ApplyNPRMaterialToMonster(this, static_cast<uint8>(EnemyType));

	// 创建描边 (Inverted Hull)
	UNPRMaterialUtils::SpawnOutlineAttachment(this, FLinearColor::Black, 2.0f);
}
