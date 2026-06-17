// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Game Mode — 格斗萌主

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FirstGameGameMode.generated.h"

UCLASS()
class FIRSTGAME_API AFirstGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFirstGameGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<class APlayerCharacter> PlayerCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "Dungeon")
	int32 TotalRooms = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Dungeon")
	int32 CurrentRoomIndex = 0;
};
