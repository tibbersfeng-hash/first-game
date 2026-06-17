// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Combo Manager — tracks and validates combo chains

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComboManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboChanged, int32, Count, FName, LastAttackType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboDropped, int32, FinalCount);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class FIRSTGAME_API UComboManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UComboManager();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ─── Combo State ────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Combo")
	int32 GetCurrentCount() const { return CurrentCount; }

	UFUNCTION(BlueprintCallable, Category = "Combo")
	FName GetLastAttackType() const { return LastAttackType; }

	UFUNCTION(BlueprintCallable, Category = "Combo")
	bool IsInCombo() const { return CurrentCount > 0; }

	// ─── Combo Operations ──────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void RegisterHit(FName AttackType);

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ResetCombo();

	UFUNCTION(BlueprintCallable, Category = "Combo")
	int32 GetNextComboIndex() const;

	// ─── Configuration ─────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	float ComboWindowTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	int32 MaxComboCount = 99;

	// ─── Events ────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Combo")
	FOnComboChanged OnComboChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combo")
	FOnComboDropped OnComboDropped;

private:
	UPROPERTY()
	int32 CurrentCount = 0;

	UPROPERTY()
	FName LastAttackType;

	UPROPERTY()
	float ComboTimer = 0.f;

	UPROPERTY()
	TArray<FName> ComboHistory;
};
