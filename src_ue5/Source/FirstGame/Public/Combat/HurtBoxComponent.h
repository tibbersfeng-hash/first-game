// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// HurtBox — Damage receiving component

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "HurtBoxComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHurtBoxDamaged, AActor*, Target, float, Damage);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class FIRSTGAME_API UHurtBoxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UHurtBoxComponent();

	UFUNCTION(BlueprintCallable, Category = "HurtBox")
	void SetInvincible(bool bInInvincible) { bInvincible = bInInvincible; }

	UFUNCTION(BlueprintCallable, Category = "HurtBox")
	bool IsInvincible() const { return bInvincible; }

	UPROPERTY(BlueprintAssignable, Category = "HurtBox")
	FOnHurtBoxDamaged OnDamaged;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnDamageOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY()
	bool bInvincible = false;
};
