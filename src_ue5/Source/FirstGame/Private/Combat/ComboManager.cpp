// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Combat/ComboManager.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "Subsystems/CombatDataSubsystem.h"

UComboManager::UComboManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentCount = 0;
	ComboTimer = 0.f;
}

void UComboManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Combo window timeout
	if (CurrentCount > 0)
	{
		ComboTimer -= DeltaTime;
		if (ComboTimer <= 0.f)
		{
			UE_LOG(LogTemp, Log, TEXT("Combo dropped at %d"), CurrentCount);
			OnComboDropped.Broadcast(CurrentCount);
			ResetCombo();
		}
	}
}

void UComboManager::RegisterHit(FName AttackType)
{
	// Check if this continues the combo or starts a new one
	bool bIsContinuation = (CurrentCount > 0 && ComboTimer > 0.f);

	LastAttackType = AttackType;
	ComboHistory.Add(AttackType);

	if (bIsContinuation)
	{
		CurrentCount++;
	}
	else
	{
		CurrentCount = 1;
		ComboHistory.Reset();
		ComboHistory.Add(AttackType);
	}

	// Reset combo window
	ComboTimer = ComboWindowTime;

	// Clamp
	if (CurrentCount > MaxComboCount)
	{
		CurrentCount = MaxComboCount;
	}

	UE_LOG(LogTemp, Log, TEXT("Combo: %d (Attack: %s)"), CurrentCount, *AttackType.ToString());

	OnComboChanged.Broadcast(CurrentCount, LastAttackType);

	// Also broadcast via SignalBus
	USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (SignalBus)
	{
		SignalBus->OnComboUpdated.Broadcast(GetOwner(), CurrentCount);
	}
}

void UComboManager::ResetCombo()
{
	if (CurrentCount > 0)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Combo reset (was %d)"), CurrentCount);
	}
	CurrentCount = 0;
	LastAttackType = NAME_None;
	ComboTimer = 0.f;
	ComboHistory.Reset();
}

int32 UComboManager::GetNextComboIndex() const
{
	// Returns the index for the next attack in the chain (0-based)
	return FMath::Clamp(CurrentCount, 0, 2); // light1=0, light2=1, light3=2
}
