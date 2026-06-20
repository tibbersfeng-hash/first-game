// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EquipmentAsset.generated.h"

/**
 * 装备类型枚举
 */
UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	Weapon      UMETA(DisplayName = "武器"),
	Armor       UMETA(DisplayName = "护甲"),
	Accessory   UMETA(DisplayName = "饰品"),
	Boots       UMETA(DisplayName = "靴子")
};

/**
 * 装备稀有度
 */
UENUM(BlueprintType)
enum class EEquipmentRarity : uint8
{
	Common      UMETA(DisplayName = "普通"),
	Uncommon    UMETA(DisplayName = "优秀"),
	Rare        UMETA(DisplayName = "稀有"),
	Epic        UMETA(DisplayName = "史诗"),
	Legendary   UMETA(DisplayName = "传说")
};

/**
 * 装备属性加成
 */
USTRUCT(BlueprintType)
struct FEquipmentStatBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxEnergy = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Defense = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MoveSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CritChance = 0.f;
};

/**
 * 装备数据资产
 * 定义装备的基本属性和加成
 */
UCLASS(BlueprintType, Blueprintable)
class FIRSTGAME_API UEquipmentAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UEquipmentAsset();

	/** 装备名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText EquipmentName;

	/** 装备类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	EEquipmentType EquipmentType;

	/** 装备稀有度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	EEquipmentRarity Rarity;

	/** 装备等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	int32 ItemLevel = 1;

	/** 属性加成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FEquipmentStatBonus StatBonus;

	/** 装备图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UTexture2D* Icon;

	/** 装备描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText Description;

	/** 获取稀有度颜色 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FLinearColor GetRarityColor() const;
};
