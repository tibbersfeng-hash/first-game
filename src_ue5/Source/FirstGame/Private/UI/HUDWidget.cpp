// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "UI/HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Subsystems/SignalBusFunctionLibrary.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Note: DYNAMIC_MULTICAST_DELEGATE::AddDynamic requires UFUNCTION member pointers, not lambdas.
	// Event binding is done in Blueprint by connecting to the SignalBus events.
	// This C++ implementation logs construction; actual wiring happens via Blueprint.

	UE_LOG(LogTemp, Log, TEXT("HUD Widget constructed — bind SignalBus events in Blueprint"));
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
