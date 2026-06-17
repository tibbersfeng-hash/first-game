// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Damage Number Widget — floating damage display

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageNumberWidget.generated.h"

class UTextBlock;

UCLASS()
class FIRSTGAME_API UDamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Damage Number")
	void SetupDamage(float InDamage, bool bInCritical = false);

	UFUNCTION(BlueprintCallable, Category = "Damage Number")
	void AnimateAndDestroy();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageText;

private:
	UPROPERTY()
	float DamageAmount = 0.f;

	UPROPERTY()
	bool bIsCritical = false;
};
