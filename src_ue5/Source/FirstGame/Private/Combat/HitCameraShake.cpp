// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Combat/HitCameraShake.h"
#include "FirstGame.h"

// ─── UHitCameraShake ─────────────────────────────────────────────────

UHitCameraShake::UHitCameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Duration and shake pattern are configured via Blueprint or CDO
}

// ─── UScreenFlashComponent ───────────────────────────────────────────

UScreenFlashComponent::UScreenFlashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsFlashing = false;
	FlashTimer = 0.f;
}

void UScreenFlashComponent::TriggerFlash(FLinearColor Color, float Duration, float MaxAlpha)
{
	FlashColor = Color;
	FlashTimer = Duration;
	MaxFlashAlpha = MaxAlpha;
	bIsFlashing = true;

	OnFlash.Broadcast(Color, MaxAlpha);

	UE_LOG(LogTemp, Log, TEXT("Screen Flash: Color=(%.1f,%.1f,%.1f), Duration=%.2f"),
		Color.R, Color.G, Color.B, Duration);
}

void UScreenFlashComponent::StopFlash()
{
	bIsFlashing = false;
	FlashTimer = 0.f;
	OnFlash.Broadcast(FLinearColor::Black, 0.f);
}
