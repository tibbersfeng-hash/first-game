// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Dungeon Room — single combat arena

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonRoom.generated.h"

class ABaseEnemy;

UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Treasure UMETA(DisplayName = "Treasure"),
	Boss UMETA(DisplayName = "Boss"),
	Rest UMETA(DisplayName = "Rest")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomCleared, class ADungeonRoom*, Room);

UCLASS()
class FIRSTGAME_API ADungeonRoom : public AActor
{
	GENERATED_BODY()

public:
	ADungeonRoom();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	ERoomType RoomType = ERoomType::Combat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	int32 RoomIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	TArray<TSubclassOf<ABaseEnemy>> EnemyClasses;

	UPROPERTY(BlueprintReadOnly, Category = "Room")
	bool bIsCleared = false;

	UPROPERTY(BlueprintAssignable, Category = "Room")
	FOnRoomCleared OnRoomCleared;

	UFUNCTION(BlueprintCallable, Category = "Room")
	void StartRoom();

	UFUNCTION(BlueprintCallable, Category = "Room")
	void CheckRoomClear();

	UFUNCTION(BlueprintCallable, Category = "Room")
	void ClearRoom();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnEnemyDied(class ABaseEnemy* Enemy);

	UPROPERTY()
	TArray<ABaseEnemy*> SpawnedEnemies;
};
