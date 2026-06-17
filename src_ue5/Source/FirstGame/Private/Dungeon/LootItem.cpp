// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Dungeon/LootItem.h"
#include "Characters/PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// ─── ALootItem ─────────────────────────────────────────────────────────

ALootItem::ALootItem()
{
	PrimaryActorTick.bCanEverTick = true;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetSphereRadius(40.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);
	RootComponent = PickupSphere;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALootItem::BeginPlay()
{
	Super::BeginPlay();
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ALootItem::OnOverlapBegin);
	RemainingLifetime = 10.f;
}

void ALootItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Bob animation
	BobTimer += DeltaTime;
	float BobOffset = FMath::Sin(BobTimer * 3.f) * 5.f;
	MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, BobOffset));

	// Lifetime countdown
	RemainingLifetime -= DeltaTime;
	if (RemainingLifetime <= 0.f)
	{
		Destroy();
	}
}

void ALootItem::SetupLoot(ELootType InType, float InValue)
{
	ItemType = InType;
	ItemValue = InValue;

	// Set mesh color based on type
	if (UMaterialInstanceDynamic* DynMat = MeshComponent->CreateAndSetMaterialInstanceDynamic(0))
	{
		switch (ItemType)
		{
		case ELootType::Health:  DynMat->SetVectorParameterValue("BaseColor", FLinearColor(0.f, 1.f, 0.f)); break;
		case ELootType::Energy:  DynMat->SetVectorParameterValue("BaseColor", FLinearColor(0.f, 0.5f, 1.f)); break;
		case ELootType::Coin:    DynMat->SetVectorParameterValue("BaseColor", FLinearColor(1.f, 0.84f, 0.f)); break;
		case ELootType::PowerUp: DynMat->SetVectorParameterValue("BaseColor", FLinearColor(1.f, 0.f, 1.f)); break;
		}
	}
}

void ALootItem::TryPickup(APlayerCharacter* Player)
{
	if (!Player) return;

	switch (ItemType)
	{
	case ELootType::Health:
		Player->Heal(ItemValue);
		break;
	case ELootType::Energy:
		Player->AddEnergy(ItemValue);
		break;
	case ELootType::Coin:
		// Coin pickup — would add to currency system
		break;
	case ELootType::PowerUp:
		// Power up — would apply temporary buff
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("Loot picked up: Type=%d, Value=%.0f"), (int)ItemType, ItemValue);

	OnPickedUp.Broadcast(Player, ItemType);

	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnShowDamageNumber.Broadcast(
			FDamageNumberData{GetActorLocation(), ItemValue, false,
				ItemType == ELootType::Health ? FLinearColor(0.f, 1.f, 0.f) : FLinearColor(0.f, 0.5f, 1.f)});
	}

	Destroy();
}

void ALootItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player)
	{
		TryPickup(Player);
	}
}

// ─── ULootTable ────────────────────────────────────────────────────────

TArray<FLootDropConfig> ULootTable::RollDrops() const
{
	TArray<FLootDropConfig> Result;

	for (const FLootDropConfig& Config : Drops)
	{
		if (FMath::FRand() <= Config.DropChance)
		{
			Result.Add(Config);
		}
	}

	return Result;
}
