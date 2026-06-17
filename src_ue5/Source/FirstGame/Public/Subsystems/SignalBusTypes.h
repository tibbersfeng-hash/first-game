// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SignalBusTypes.generated.h"

/** Damage number display data */
USTRUCT(BlueprintType)
struct FDamageNumberData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector WorldLocation;

	UPROPERTY(BlueprintReadWrite)
	float Damage;

	UPROPERTY(BlueprintReadWrite)
	bool bIsCritical;

	UPROPERTY(BlueprintReadWrite)
	FLinearColor TextColor;

	FDamageNumberData()
		: WorldLocation(FVector::ZeroVector)
		, Damage(0.f)
		, bIsCritical(false)
		, TextColor(FLinearColor::White)
	{}

	FDamageNumberData(FVector InLocation, float InDamage, bool InIsCritical, FLinearColor InColor)
		: WorldLocation(InLocation)
		, Damage(InDamage)
		, bIsCritical(InIsCritical)
		, TextColor(InColor)
	{}
};
