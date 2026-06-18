// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Character Data Factory — creates default character DataAssets from GDD values

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CharacterDataFactory.generated.h"

class UCharacterDataAsset;

/**
 * Provides default character data values from the GDD.
 * Called during editor initialization or via commandlet to create DataAssets.
 *
 * GDD Source: design/gdd/combat-system.md, design/gdd/player-controller.md
 */
UCLASS()
class FIRSTGAME_API UCharacterDataFactory : public UObject
{
	GENERATED_BODY()

public:
	/** Create default data for Huikong (气功小武僧) */
	UFUNCTION(BlueprintCallable, Category = "Data Factory")
	static UCharacterDataAsset* CreateHuikongData(UObject* Outer);

	/** Create default data for Tangtang (糖果魔导师) */
	UFUNCTION(BlueprintCallable, Category = "Data Factory")
	static UCharacterDataAsset* CreateTangtangData(UObject* Outer);

	/** Create default data for Kiguemaru (萌鬼狂战士) */
	UFUNCTION(BlueprintCallable, Category = "Data Factory")
	static UCharacterDataAsset* CreateKiguemaruData(UObject* Outer);

	/** Create default enemy data (CandyZombie-like, for generic enemies without specific class) */
	UFUNCTION(BlueprintCallable, Category = "Data Factory")
	static UCharacterDataAsset* CreateDefaultEnemyData(UObject* Outer);

	/** Get default light attack values from GDD */
	static struct FAttackMoveData GetDefaultLightAttack(int32 Index);

	/** Get default heavy attack values from GDD */
	static struct FAttackMoveData GetDefaultHeavyAttack();

	/** Get default special move values from GDD */
	static struct FAttackMoveData GetDefaultSpecialMove();
};
