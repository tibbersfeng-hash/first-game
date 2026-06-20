// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Equipment/EquipmentManagerComponent.h"
#include "FirstGame.h"

UEquipmentManagerComponent::UEquipmentManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// 初始化装备槽位
	FEquipmentSlot WeaponSlot;
	WeaponSlot.SlotType = EEquipmentType::Weapon;
	EquipmentSlots.Add(WeaponSlot);

	FEquipmentSlot ArmorSlot;
	ArmorSlot.SlotType = EEquipmentType::Armor;
	EquipmentSlots.Add(ArmorSlot);

	FEquipmentSlot AccessorySlot;
	AccessorySlot.SlotType = EEquipmentType::Accessory;
	EquipmentSlots.Add(AccessorySlot);

	FEquipmentSlot BootsSlot;
	BootsSlot.SlotType = EEquipmentType::Boots;
	EquipmentSlots.Add(BootsSlot);
}

void UEquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("EquipmentManager: Initialized with %d slots"), EquipmentSlots.Num());
}

bool UEquipmentManagerComponent::EquipItem(UEquipmentAsset* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentManager: Try to equip null item"));
		return false;
	}

	// 找到对应类型的槽位
	for (FEquipmentSlot& Slot : EquipmentSlots)
	{
		if (Slot.SlotType == Item->EquipmentType)
		{
			// 如果槽位已有物品，先卸下
			if (Slot.EquippedItem)
			{
				UnequipItem(Slot.SlotType);
			}

			// 装备新物品
			Slot.EquippedItem = Item;
			RecalculateStats();

			UE_LOG(LogTemp, Log, TEXT("EquipmentManager: Equipped %s"), *Item->EquipmentName.ToString());

			// 广播事件
			OnEquipmentChanged.Broadcast(Item->EquipmentType, Item);

			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("EquipmentManager: No slot for type %d"), (int)Item->EquipmentType);
	return false;
}

bool UEquipmentManagerComponent::UnequipItem(EEquipmentType SlotType)
{
	for (FEquipmentSlot& Slot : EquipmentSlots)
	{
		if (Slot.SlotType == SlotType)
		{
			if (Slot.EquippedItem)
			{
				UEquipmentAsset* OldItem = Slot.EquippedItem;
				Slot.EquippedItem = nullptr;
				RecalculateStats();

				UE_LOG(LogTemp, Log, TEXT("EquipmentManager: Unequipped %s"), *OldItem->EquipmentName.ToString());

				OnEquipmentChanged.Broadcast(SlotType, nullptr);

				return true;
			}
			return false;
		}
	}
	return false;
}

UEquipmentAsset* UEquipmentManagerComponent::GetEquippedItem(EEquipmentType SlotType) const
{
	for (const FEquipmentSlot& Slot : EquipmentSlots)
	{
		if (Slot.SlotType == SlotType)
		{
			return Slot.EquippedItem;
		}
	}
	return nullptr;
}

FEquipmentStatBonus UEquipmentManagerComponent::GetTotalStatBonus() const
{
	FEquipmentStatBonus TotalBonus;

	for (const FEquipmentSlot& Slot : EquipmentSlots)
	{
		if (Slot.EquippedItem)
		{
			TotalBonus.MaxHealth += Slot.EquippedItem->StatBonus.MaxHealth;
			TotalBonus.MaxEnergy += Slot.EquippedItem->StatBonus.MaxEnergy;
			TotalBonus.AttackDamage += Slot.EquippedItem->StatBonus.AttackDamage;
			TotalBonus.Defense += Slot.EquippedItem->StatBonus.Defense;
			TotalBonus.MoveSpeed += Slot.EquippedItem->StatBonus.MoveSpeed;
			TotalBonus.CritChance += Slot.EquippedItem->StatBonus.CritChance;
		}
	}

	return TotalBonus;
}

int32 UEquipmentManagerComponent::GetEquippedCount() const
{
	int32 Count = 0;
	for (const FEquipmentSlot& Slot : EquipmentSlots)
	{
		if (Slot.EquippedItem)
		{
			Count++;
		}
	}
	return Count;
}

void UEquipmentManagerComponent::RecalculateStats()
{
	FEquipmentStatBonus TotalBonus = GetTotalStatBonus();

	UE_LOG(LogTemp, Log, TEXT("EquipmentManager: Total Stats - HP:+%.0f, DMG:+%.0f, DEF:+%.0f"),
		TotalBonus.MaxHealth, TotalBonus.AttackDamage, TotalBonus.Defense);

	// 这里可以应用属性加成到角色
	// 通过 SignalBus 通知角色系统更新属性
}
