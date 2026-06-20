// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "DataAssets/EquipmentAsset.h"

UEquipmentAsset::UEquipmentAsset()
{
	// 默认值
	EquipmentName = FText::FromString(TEXT("未命名装备"));
	EquipmentType = EEquipmentType::Weapon;
	Rarity = EEquipmentRarity::Common;
	ItemLevel = 1;
}

FLinearColor UEquipmentAsset::GetRarityColor() const
{
	switch (Rarity)
	{
	case EEquipmentRarity::Common:
		return FLinearColor(0.7f, 0.7f, 0.7f, 1.f);  // 灰色
	case EEquipmentRarity::Uncommon:
		return FLinearColor(0.0f, 0.8f, 0.0f, 1.f);  // 绿色
	case EEquipmentRarity::Rare:
		return FLinearColor(0.0f, 0.5f, 1.0f, 1.f);  // 蓝色
	case EEquipmentRarity::Epic:
		return FLinearColor(0.6f, 0.0f, 1.0f, 1.f);  // 紫色
	case EEquipmentRarity::Legendary:
		return FLinearColor(1.0f, 0.6f, 0.0f, 1.f);  // 橙色
	default:
		return FLinearColor::White;
	}
}
