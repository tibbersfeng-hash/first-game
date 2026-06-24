// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// HUD Widget — UMG-based heads-up display (SignalBus 订阅)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class USignalBusSubsystem;

UCLASS()
class FIRSTGAME_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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

	// ─── Enemy Lock-On HUD ─────────────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* EnemyHealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* EnemyNameText;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHealth(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateEnergy(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateCombo(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateRoom(int32 Current, int32 Total);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateEnemyHealth(const FString& Name, float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideEnemyHealth();

	// ─── SignalBus 事件回调 ───────────────────────────────────────
	UFUNCTION()
	void OnPlayerHealthChanged(class AActor* Player, float NewHealth);

	UFUNCTION()
	void OnPlayerEnergyChanged(class AActor* Player, float NewEnergy);

	UFUNCTION()
	void OnComboUpdated(class AActor* Player, int32 CurrentCount);

	UFUNCTION()
	void OnRoomEntered(class AActor* Room);

	UFUNCTION()
	void OnRoomCleared(class AActor* Room);

private:
	// SignalBus 引用 (用于解绑)
	UPROPERTY()
	USignalBusSubsystem* CachedSignalBus = nullptr;

	// 房间进度跟踪
	int32 CurrentRoomIndex = 0;
	int32 TotalRooms = 1;
};
