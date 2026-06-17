// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// HUD Widget — UMG-based heads-up display

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

UCLASS()
class FIRSTGAME_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// ─── Health Bar ──────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	// ── Energy Bar ──────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	UProgressBar* EnergyBar;

	// ─── Combo Counter ──────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ComboText;

	// ─── Room Info ──────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoomText;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHealth(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateEnergy(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateCombo(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateRoom(int32 Current, int32 Total);
};
