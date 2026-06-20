// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/EquipmentAsset.h"
#include "EquipmentManagerComponent.generated.h"

/**
 * 装备槽位
 */
USTRUCT(BlueprintType)
struct FEquipmentSlot
{
	GENERATED_BODY()

	UPROPERTY()
	EEquipmentType SlotType = EEquipmentType::Weapon;

	UPROPERTY()
	UEquipmentAsset* EquippedItem = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChanged, EEquipmentType, SlotType, UEquipmentAsset*, Item);

/**
 * 装备管理器组件
 * 管理角色的装备槽位和属性加成
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FIRSTGAME_API UEquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentManagerComponent();

	virtual void BeginPlay() override;

	/** 装备物品 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipItem(UEquipmentAsset* Item);

	/** 卸下物品 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool UnequipItem(EEquipmentType SlotType);

	/** 获取槽位中的物品 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UEquipmentAsset* GetEquippedItem(EEquipmentType SlotType) const;

	/** 获取所有属性加成 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FEquipmentStatBonus GetTotalStatBonus() const;

	/** 装备数量 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	int32 GetEquippedCount() const;

	/** 装备变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnEquipmentChanged OnEquipmentChanged;

protected:
	/** 装备槽位 */
	UPROPERTY()
	TArray<FEquipmentSlot> EquipmentSlots;

private:
	/** 重新计算属性加成 */
	void RecalculateStats();
};
