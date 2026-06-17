// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Screen Shake — camera shake on hit/combo/special

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "HitCameraShake.generated.h"

/**
 * Camera shake preset for combat impacts.
 * Configurable via Duration in constructor.
 * Actual shake pattern is configured in Blueprint or CDO.
 */
UCLASS(Blueprintable)
class FIRSTGAME_API UHitCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	UHitCameraShake(const FObjectInitializer& ObjectInitializer);

	/** Oscillation amplitude (how far the camera moves) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shake")
	float OscillationAmplitude = 5.f;

	/** Oscillation frequency (how fast it shakes) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shake")
	float OscillationFrequency = 30.f;
};

/**
 * Screen flash effect on critical hits and special moves.
 */
UCLASS()
class FIRSTGAME_API UScreenFlashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScreenFlashComponent();

	UFUNCTION(BlueprintCallable, Category = "Screen Flash")
	void TriggerFlash(FLinearColor Color = FLinearColor::White, float Duration = 0.1f, float MaxAlpha = 0.3f);

	UFUNCTION(BlueprintCallable, Category = "Screen Flash")
	void StopFlash();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScreenFlash, FLinearColor, Color, float, Alpha);

	UPROPERTY(BlueprintAssignable, Category = "Screen Flash")
	FOnScreenFlash OnFlash;

private:
	UPROPERTY()
	float FlashTimer = 0.f;

	UPROPERTY()
	float MaxFlashAlpha = 0.3f;

	UPROPERTY()
	FLinearColor FlashColor = FLinearColor::White;

	UPROPERTY()
	bool bIsFlashing = false;
};
