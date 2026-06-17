// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// HitBox — Attack detection component

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "HitBoxComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHitBoxHit, AActor*, Attacker, AActor*, Target, float, Damage);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class FIRSTGAME_API UHitBoxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UHitBoxComponent();

	UFUNCTION(BlueprintCallable, Category = "HitBox")
	void ActivateHitBox(float InDamage, AActor* InAttacker);

	UFUNCTION(BlueprintCallable, Category = "HitBox")
	void DeactivateHitBox();

	UFUNCTION(BlueprintCallable, Category = "HitBox")
	void SetKnockback(FVector2D KnockbackDir) { Knockback = KnockbackDir; }

	UFUNCTION(BlueprintCallable, Category = "HitBox")
	void SetLaunch(float InForce) { LaunchForce = InForce; }

	UPROPERTY(BlueprintAssignable, Category = "HitBox")
	FOnHitBoxHit OnHit;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnHitBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY()
	float CurrentDamage = 0.f;

	UPROPERTY()
	AActor* CurrentAttacker = nullptr;

	UPROPERTY()
	FVector2D Knockback = FVector2D(100.f, -50.f);

	UPROPERTY()
	float LaunchForce = 0.f;

	UPROPERTY()
	bool bHitBoxEnabled = false;
};
