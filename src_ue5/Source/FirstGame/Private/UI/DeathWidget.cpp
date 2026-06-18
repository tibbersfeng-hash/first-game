// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "UI/DeathWidget.h"
#include "FirstGame.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Characters/PlayerCharacter.h"
#include "DataAssets/CharacterDataAsset.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SignalBusSubsystem.h"
#include "Subsystems/SignalBusFunctionLibrary.h"

void UDeathWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RespawnButton)
	{
		RespawnButton->OnClicked.AddDynamic(this, &UDeathWidget::OnRespawnClicked);
	}

	// 默认文本
	if (DeathText)
	{
		DeathText->SetText(FText::FromString(TEXT("你死了")));
	}
	if (RespawnHint)
	{
		RespawnHint->SetText(FText::FromString(TEXT("按空格或点击按钮重生")));
	}

	UE_LOG(LogTemp, Log, TEXT("DeathWidget: 构造完成"));
}

void UDeathWidget::OnRespawnClicked()
{
	UE_LOG(LogTemp, Log, TEXT("DeathWidget: 重生按钮点击"));

	// 找到玩家
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("DeathWidget: 找不到 PlayerCharacter"));
		return;
	}

	// 重置玩家状态
	Player->CurrentHealth = Player->CharacterData ? Player->CharacterData->MaxHealth : 100.f;
	Player->CurrentEnergy = Player->CharacterData ? Player->CharacterData->MaxEnergy : 100.f;
	Player->CurrentComboCount = 0;

	// 广播血量/能量变化 (HUD 更新)
	if (USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this))
	{
		SignalBus->OnPlayerHealthChanged.Broadcast(Player, Player->CurrentHealth);
		SignalBus->OnPlayerEnergyChanged.Broadcast(Player, Player->CurrentEnergy);
	}

	// 销毁 UI
	RemoveFromParent();

	UE_LOG(LogTemp, Log, TEXT("DeathWidget: 玩家重生完成"));
}

void UDeathWidget::SetDeathInfo(int32 RoomIndex, int32 ComboCount)
{
	if (DeathText)
	{
		FString Info = FString::Printf(TEXT("你死了\n房间 %d | 最高连击 %d"), RoomIndex + 1, ComboCount);
		DeathText->SetText(FText::FromString(Info));
	}
}
