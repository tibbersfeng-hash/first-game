// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Game Instance — registers all subsystems

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FirstGameGameInstance.generated.h"

UCLASS()
class FIRSTGAME_API UFirstGameGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
};
