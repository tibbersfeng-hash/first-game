// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Dungeon Flow — manages room progression

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonFlow.generated.h"

class ADungeonRoom;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonFlowRoomChanged, int32, RoomIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonFlowCompleted, bool, bVictory);

UCLASS()
class FIRSTGAME_API ADungeonFlow : public AActor
{
	GENERATED_BODY()

public:
	ADungeonFlow();

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void InitializeDungeon(TArray<ADungeonRoom*>& InRooms);

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void StartDungeon();

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void AdvanceToNextRoom();

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	int32 GetCurrentRoomIndex() const { return CurrentRoomIndex; }

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	int32 GetTotalRooms() const { return Rooms.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	ADungeonRoom* GetCurrentRoom() const;

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	bool IsDungeonComplete() const { return bIsComplete; }

	UPROPERTY(BlueprintAssignable, Category = "Dungeon")
	FOnDungeonFlowRoomChanged OnRoomChanged;

	UPROPERTY(BlueprintAssignable, Category = "Dungeon")
	FOnDungeonFlowCompleted OnDungeonCompleted;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRoomCleared(ADungeonRoom* Room);

	UPROPERTY()
	TArray<ADungeonRoom*> Rooms;

	UPROPERTY()
	int32 CurrentRoomIndex = -1;

	UPROPERTY()
	bool bIsComplete = false;
};
