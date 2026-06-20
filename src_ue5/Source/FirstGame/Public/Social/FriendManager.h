// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FriendManager.generated.h"

/**
 * 好友在线状态
 */
UENUM(BlueprintType)
enum class EFriendStatus : uint8
{
	Offline     UMETA(DisplayName = "离线"),
	Online      UMETA(DisplayName = "在线"),
	InGame      UMETA(DisplayName = "游戏中"),
	InMatch     UMETA(DisplayName = "匹配中")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFriendStatusChanged, FString, PlayerID, EFriendStatus, NewStatus);

/**
 * 好友数据结构
 */
USTRUCT(BlueprintType)
struct FFriendData
{
	GENERATED_BODY()

	/** 玩家 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
	FString PlayerID;

	/** 玩家名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
	FString PlayerName;

	/** 好友状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
	EFriendStatus Status;

	/** 段位等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
	int32 RankLevel = 0;

	/** 最后在线时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
	FDateTime LastOnline;

	/** 是否为好友 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
	bool bIsFriend = false;

	/** 是否被拉黑 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
	bool bIsBlocked = false;

	FORCEINLINE bool operator==(const FFriendData& Other) const
	{
		return PlayerID == Other.PlayerID;
	}

	FORCEINLINE bool operator!=(const FFriendData& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * 好友列表管理器
 */
UCLASS(BlueprintType, Blueprintable)
class FIRSTGAME_API UFriendManager : public UObject
{
	GENERATED_BODY()

public:
	UFriendManager();

	/** 添加好友 */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	bool AddFriend(FString PlayerID, FString PlayerName);

	/** 删除好友 */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	bool RemoveFriend(FString PlayerID);

	/** 拉黑玩家 */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	bool BlockPlayer(FString PlayerID);

	/** 获取好友列表 */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	TArray<FFriendData> GetFriendsList() const;

	/** 获取在线好友 */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	TArray<FFriendData> GetOnlineFriends() const;

	/** 获取好友数量 */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	int32 GetFriendCount() const;

	/** 更新好友状态 */
	UFUNCTION(BlueprintCallable, Category = "Friends")
	void UpdateFriendStatus(FString PlayerID, EFriendStatus NewStatus);

	/** 好友状态变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "Friends")
	FOnFriendStatusChanged OnFriendStatusChanged;

protected:
	/** 好友列表 */
	UPROPERTY()
	TArray<FFriendData> FriendsList;

private:
	/** 查找好友 */
	FFriendData* FindFriend(FString PlayerID);
};
