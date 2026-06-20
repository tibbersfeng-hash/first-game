// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "UI/DamageNumberWidget.h"
#include "FirstGame.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UDamageNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDamageNumberWidget::SetupDamage(float InDamage, bool bInCritical)
{
	DamageAmount = InDamage;
	bIsCritical = bInCritical;

	if (DamageText)
	{
		FString Text = FString::Printf(TEXT("%.0f"), InDamage);
		if (bInCritical)
		{
			Text = FString::Printf(TEXT("CRIT %s!"), *Text);
			DamageText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.3f, 0.1f)));
		}
		else
		{
			DamageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
		DamageText->SetText(FText::FromString(Text));
	}
}

void UDamageNumberWidget::AnimateAndDestroy()
{
	// In production: play Float Up + Fade Out animation
	// For now, destroy after delay
	if (UWorld* World = GetWorld())
	{
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [this]()
		{
			RemoveFromParent();
		}, 1.0f, false);
	}
}
