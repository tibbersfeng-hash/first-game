// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Death Widget — 玩家死亡时显示的 UI

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class FIRSTGAME_API UDeathWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeathText;

	UPROPERTY(meta = (BindWidget))
	UButton* RespawnButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RespawnHint;

	UFUNCTION()
	void OnRespawnClicked();

public:
	UFUNCTION(BlueprintCallable, Category = "Death")
	void SetDeathInfo(int32 RoomIndex, int32 ComboCount);
};
