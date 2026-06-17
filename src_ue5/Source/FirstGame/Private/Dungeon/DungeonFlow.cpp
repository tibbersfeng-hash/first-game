// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Dungeon/DungeonFlow.h"
#include "Dungeon/DungeonRoom.h"
#include "Subsystems/SignalBusFunctionLibrary.h"

ADungeonFlow::ADungeonFlow()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADungeonFlow::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("DungeonFlow initialized"));
}

void ADungeonFlow::InitializeDungeon(TArray<ADungeonRoom*>& InRooms)
{
	Rooms = InRooms;
	CurrentRoomIndex = -1;
	bIsComplete = false;

	for (int32 i = 0; i < Rooms.Num(); i++)
	{
		if (Rooms[i])
		{
			Rooms[i]->OnRoomCleared.AddDynamic(this, &ADungeonFlow::OnRoomCleared);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Dungeon initialized with %d rooms"), Rooms.Num());
}

void ADungeonFlow::StartDungeon()
{
	if (Rooms.Num() == 0) return;
	CurrentRoomIndex = 0;

	ADungeonRoom* FirstRoom = GetCurrentRoom();
	if (FirstRoom)
	{
		FirstRoom->StartRoom();
		OnRoomChanged.Broadcast(CurrentRoomIndex);

		USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
		if (SignalBus)
		{
			SignalBus->OnRoomEntered.Broadcast(FirstRoom);
		}
	}
}

void ADungeonFlow::AdvanceToNextRoom()
{
	CurrentRoomIndex++;

	if (CurrentRoomIndex >= Rooms.Num())
	{
		bIsComplete = true;
		OnDungeonCompleted.Broadcast(true);

		USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
		if (SignalBus)
		{
			SignalBus->OnDungeonCompleted.Broadcast(true);
		}
		return;
	}

	ADungeonRoom* NextRoom = GetCurrentRoom();
	if (NextRoom)
	{
		NextRoom->StartRoom();
		OnRoomChanged.Broadcast(CurrentRoomIndex);

		USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
		if (SignalBus)
		{
			SignalBus->OnRoomEntered.Broadcast(NextRoom);
		}
	}
}

ADungeonRoom* ADungeonFlow::GetCurrentRoom() const
{
	if (CurrentRoomIndex < 0 || CurrentRoomIndex >= Rooms.Num()) return nullptr;
	return Rooms[CurrentRoomIndex];
}

void ADungeonFlow::OnRoomCleared(ADungeonRoom* Room)
{
	UE_LOG(LogTemp, Log, TEXT("Room %d cleared, advancing..."), CurrentRoomIndex);
	AdvanceToNextRoom();
}
