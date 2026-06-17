// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// HitStop Manager — frame freeze effect on hit

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitStopManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitStopActive, float, RemainingTime);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class FIRSTGAME_API UHitStopManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitStopManager();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "HitStop")
	void RequestHitStop(float Duration);

	UFUNCTION(BlueprintCallable, Category = "HitStop")
	bool IsInHitStop() const { return bHitStopEnabled; }

	UFUNCTION(BlueprintCallable, Category = "HitStop")
	float GetRemainingTime() const { return RemainingTime; }

	UPROPERTY(BlueprintAssignable, Category = "HitStop")
	FOnHitStopActive OnHitStopTick;

	UPROPERTY(EditDefaultsOnly, Category = "HitStop")
	float TimeDilationDuringHitStop = 0.01f;

	UPROPERTY(EditDefaultsOnly, Category = "HitStop")
	bool bAffectGlobalTimeDilation = true;

private:
	UPROPERTY()
	bool bHitStopEnabled = false;

	UPROPERTY()
	float RemainingTime = 0.f;
};
