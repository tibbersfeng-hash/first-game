// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "UI/HUDWidget.h"
#include "FirstGame.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Subsystems/SignalBusSubsystem.h"
#include "Subsystems/SignalBusFunctionLibrary.h"
#include "Characters/PlayerCharacter.h"
#include "DataAssets/CharacterDataAsset.h"
#include "Dungeon/DungeonRoom.h"
#include "Dungeon/DungeonFlow.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 订阅 SignalBus 事件
	CachedSignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
	if (CachedSignalBus)
	{
		CachedSignalBus->OnPlayerHealthChanged.AddDynamic(this, &UHUDWidget::OnPlayerHealthChanged);
		CachedSignalBus->OnPlayerEnergyChanged.AddDynamic(this, &UHUDWidget::OnPlayerEnergyChanged);
		CachedSignalBus->OnComboUpdated.AddDynamic(this, &UHUDWidget::OnComboUpdated);
		CachedSignalBus->OnRoomEntered.AddDynamic(this, &UHUDWidget::OnRoomEntered);
		CachedSignalBus->OnRoomCleared.AddDynamic(this, &UHUDWidget::OnRoomCleared);

		UE_LOG(LogTemp, Log, TEXT("HUD Widget: SignalBus 事件已绑定"));

		// 初始显示: 从当前 PlayerCharacter 读取状态
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
		{
			float MaxHP = Player->CharacterData ? Player->CharacterData->MaxHealth : 100.f;
			float MaxEnergy = Player->CharacterData ? Player->CharacterData->MaxEnergy : 100.f;
			UpdateHealth(Player->CurrentHealth, MaxHP);
			UpdateEnergy(Player->CurrentEnergy, MaxEnergy);
			UpdateCombo(Player->CurrentComboCount);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HUD Widget: SignalBus 不可用, HUD 不会更新"));
	}
}

void UHUDWidget::NativeDestruct()
{
	// 解绑 SignalBus 事件
	if (CachedSignalBus)
	{
		CachedSignalBus->OnPlayerHealthChanged.RemoveDynamic(this, &UHUDWidget::OnPlayerHealthChanged);
		CachedSignalBus->OnPlayerEnergyChanged.RemoveDynamic(this, &UHUDWidget::OnPlayerEnergyChanged);
		CachedSignalBus->OnComboUpdated.RemoveDynamic(this, &UHUDWidget::OnComboUpdated);
		CachedSignalBus->OnRoomEntered.RemoveDynamic(this, &UHUDWidget::OnRoomEntered);
		CachedSignalBus->OnRoomCleared.RemoveDynamic(this, &UHUDWidget::OnRoomCleared);
		CachedSignalBus = nullptr;
	}

	Super::NativeDestruct();
}

void UHUDWidget::UpdateHealth(float Current, float Max)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Max > 0.f ? Current / Max : 0.f);
	}
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Current, Max)));
	}
}

void UHUDWidget::UpdateEnergy(float Current, float Max)
{
	if (EnergyBar)
	{
		EnergyBar->SetPercent(Max > 0.f ? Current / Max : 0.f);
	}
}

void UHUDWidget::UpdateCombo(int32 Count)
{
	if (ComboText)
	{
		if (Count > 0)
		{
			ComboText->SetText(FText::FromString(FString::Printf(TEXT("%d COMBO!"), Count)));
			ComboText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ComboText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UHUDWidget::UpdateRoom(int32 Current, int32 Total)
{
	if (RoomText)
	{
		RoomText->SetText(FText::FromString(FString::Printf(TEXT("Room %d / %d"), Current + 1, Total)));
	}
}

// ─── SignalBus 回调 ─────────────────────────────────────────────────

void UHUDWidget::OnPlayerHealthChanged(AActor* Player, float NewHealth)
{
	APlayerCharacter* PC = Cast<APlayerCharacter>(Player);
	if (!PC) return;

	float MaxHP = PC->CharacterData ? PC->CharacterData->MaxHealth : 100.f;
	UpdateHealth(NewHealth, MaxHP);
}

void UHUDWidget::OnPlayerEnergyChanged(AActor* Player, float NewEnergy)
{
	APlayerCharacter* PC = Cast<APlayerCharacter>(Player);
	if (!PC) return;

	float MaxEnergy = PC->CharacterData ? PC->CharacterData->MaxEnergy : 100.f;
	UpdateEnergy(NewEnergy, MaxEnergy);
}

void UHUDWidget::OnComboUpdated(AActor* Player, int32 CurrentCount)
{
	// 只更新本地玩家的 combo
	APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if (Player != PC) return;

	UpdateCombo(CurrentCount);
}

void UHUDWidget::OnRoomEntered(AActor* Room)
{
	ADungeonRoom* DR = Cast<ADungeonRoom>(Room);
	if (!DR) return;

	CurrentRoomIndex = DR->RoomIndex;
	UpdateRoom(CurrentRoomIndex, TotalRooms);

	UE_LOG(LogTemp, Log, TEXT("HUD: 进入房间 %d"), CurrentRoomIndex + 1);
}

void UHUDWidget::OnRoomCleared(AActor* Room)
{
	ADungeonRoom* DR = Cast<ADungeonRoom>(Room);
	if (!DR) return;

	UE_LOG(LogTemp, Log, TEXT("HUD: 房间 %d 通关"), DR->RoomIndex + 1);
}
