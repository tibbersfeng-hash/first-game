// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Combat/HitStopManager.h"
#include "FirstGame.h"
#include "GameFramework/WorldSettings.h"

UHitStopManager::UHitStopManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	bHitStopEnabled = false;
	RemainingTime = 0.f;
}

void UHitStopManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (IsRegistered()) { Super::TickComponent(DeltaTime, TickType, ThisTickFunction); }

	if (!bHitStopEnabled) return;

	RemainingTime -= DeltaTime;
	OnHitStopTick.Broadcast(RemainingTime);

	if (RemainingTime <= 0.f)
	{
		bHitStopEnabled = false;
		RemainingTime = 0.f;

		// Restore time dilation
		if (bAffectGlobalTimeDilation)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetWorldSettings()->SetTimeDilation(1.0f);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("HitStop ended"));
	}
}

void UHitStopManager::RequestHitStop(float Duration)
{
	// Stack hit stops — use the longer duration
	if (bHitStopEnabled && Duration <= RemainingTime) return;

	bHitStopEnabled = true;
	RemainingTime = Duration;

	// Apply global time dilation (slow everything except HitStopManager's own tick)
	if (bAffectGlobalTimeDilation)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetWorldSettings()->SetTimeDilation(TimeDilationDuringHitStop);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HitStop started: %.3f seconds"), Duration);
}
