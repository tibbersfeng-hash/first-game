// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Enhanced Input System Configuration

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "FirstGameInputConfig.generated.h"

/**
 * DataAsset that holds all input actions and mapping contexts.
 * Assigned to the player character for Enhanced Input binding.
 */
UCLASS(BlueprintType, Blueprintable)
class FIRSTGAME_API UFirstGameInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// ─── Mapping Context ─────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	// ─── Movement ────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
	UInputAction* MoveAction;

	// ─── Combat ──────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat")
	UInputAction* LightAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat")
	UInputAction* HeavyAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat")
	UInputAction* SpecialAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat")
	UInputAction* DodgeAction;

	// ─── Navigation ──────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Navigation")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Navigation")
	UInputAction* InteractAction;

	// ─── UI ──────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|UI")
	UInputAction* PauseAction;

	/**
	 * Get all input actions for binding.
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	TArray<UInputAction*> GetAllActions() const;
};
