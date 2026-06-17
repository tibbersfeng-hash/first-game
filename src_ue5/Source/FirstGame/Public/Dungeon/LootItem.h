// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Loot System — enemy death drops and pickup

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LootItem.generated.h"

class APlayerCharacter;

/** Loot item types */
UENUM(BlueprintType)
enum class ELootType : uint8
{
	Health UMETA(DisplayName = "Health Pack"),
	Energy UMETA(DisplayName = "Energy Orb"),
	Coin UMETA(DisplayName = "Coin"),
	PowerUp UMETA(DisplayName = "Power Up")
};

/** Loot drop configuration */
USTRUCT(BlueprintType)
struct FLootDropConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	ELootType LootType = ELootType::Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	float DropChance = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	float Value = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	float PickupRange = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	float Lifetime = 10.f;

	FLootDropConfig() : DropChance(0.3f), Value(20.f), PickupRange(60.f), Lifetime(10.f) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLootPickedUp, APlayerCharacter*, Player, ELootType, LootType);

/** A dropped loot item in the world */
UCLASS()
class FIRSTGAME_API ALootItem : public AActor
{
	GENERATED_BODY()

public:
	ALootItem();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SetupLoot(ELootType InType, float InValue);

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void TryPickup(APlayerCharacter* Player);

	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnLootPickedUp OnPickedUp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	ELootType ItemType = ELootType::Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	float ItemValue = 20.f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	class USphereComponent* PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	class UStaticMeshComponent* MeshComponent;

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY()
	float RemainingLifetime = 10.f;

	UPROPERTY()
	float BobTimer = 0.f;
};

/** Loot table — defines drop chances per enemy type */
UCLASS(BlueprintType, Blueprintable)
class FIRSTGAME_API ULootTable : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot Table")
	TArray<FLootDropConfig> Drops;

	UFUNCTION(BlueprintCallable, Category = "Loot Table")
	TArray<FLootDropConfig> RollDrops() const;
};
