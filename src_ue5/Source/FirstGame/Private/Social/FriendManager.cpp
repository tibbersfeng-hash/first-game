// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Social/FriendManager.h"
#include "FirstGame.h"

UFriendManager::UFriendManager()
{
}

bool UFriendManager::AddFriend(FString PlayerID, FString PlayerName)
{
	// 检查是否已存在
	if (FindFriend(PlayerID))
	{
		UE_LOG(LogTemp, Warning, TEXT("FriendManager: Player %s already in list"), *PlayerID);
		return false;
	}

	FFriendData NewFriend;
	NewFriend.PlayerID = PlayerID;
	NewFriend.PlayerName = PlayerName;
	NewFriend.Status = EFriendStatus::Offline;
	NewFriend.bIsFriend = true;

	FriendsList.Add(NewFriend);

	UE_LOG(LogTemp, Log, TEXT("FriendManager: Added friend %s"), *PlayerName);

	return true;
}

bool UFriendManager::RemoveFriend(FString PlayerID)
{
	FFriendData* Friend = FindFriend(PlayerID);
	if (!Friend)
	{
		return false;
	}

	FriendsList.Remove(*Friend);

	UE_LOG(LogTemp, Log, TEXT("FriendManager: Removed friend %s"), *Friend->PlayerName);

	return true;
}

bool UFriendManager::BlockPlayer(FString PlayerID)
{
	FFriendData* Friend = FindFriend(PlayerID);
	if (!Friend)
	{
		return false;
	}

	Friend->bIsBlocked = true;
	Friend->bIsFriend = false;

	UE_LOG(LogTemp, Log, TEXT("FriendManager: Blocked player %s"), *Friend->PlayerName);

	return true;
}

TArray<FFriendData> UFriendManager::GetFriendsList() const
{
	TArray<FFriendData> Friends;
	for (const FFriendData& Friend : FriendsList)
	{
		if (Friend.bIsFriend && !Friend.bIsBlocked)
		{
			Friends.Add(Friend);
		}
	}
	return Friends;
}

TArray<FFriendData> UFriendManager::GetOnlineFriends() const
{
	TArray<FFriendData> OnlineFriends;
	for (const FFriendData& Friend : FriendsList)
	{
		if (Friend.bIsFriend && !Friend.bIsBlocked &&
			(Friend.Status == EFriendStatus::Online || Friend.Status == EFriendStatus::InGame))
		{
			OnlineFriends.Add(Friend);
		}
	}
	return OnlineFriends;
}

int32 UFriendManager::GetFriendCount() const
{
	int32 Count = 0;
	for (const FFriendData& Friend : FriendsList)
	{
		if (Friend.bIsFriend && !Friend.bIsBlocked)
		{
			Count++;
		}
	}
	return Count;
}

void UFriendManager::UpdateFriendStatus(FString PlayerID, EFriendStatus NewStatus)
{
	FFriendData* Friend = FindFriend(PlayerID);
	if (!Friend)
	{
		return;
	}

	Friend->Status = NewStatus;

	UE_LOG(LogTemp, Log, TEXT("FriendManager: Updated %s status to %d"), *Friend->PlayerName, (int)NewStatus);

	// 广播事件
	OnFriendStatusChanged.Broadcast(Friend->PlayerID, NewStatus);
}

FFriendData* UFriendManager::FindFriend(FString PlayerID)
{
	for (FFriendData& Friend : FriendsList)
	{
		if (Friend.PlayerID == PlayerID)
		{
			return &Friend;
		}
	}
	return nullptr;
}
