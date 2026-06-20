// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// SetMaterialFlagsCommandlet — 设置材质的 bUsedWithSkeletalMesh flag

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "SetMaterialFlagsCommandlet.generated.h"

/**
 * Commandlet to set bUsedWithSkeletalMesh on materials.
 * Usage: UnrealEditor-Cmd <Project> -run=SetMaterialFlags /Game/Path/To/Material
 */
UCLASS()
class FIRSTGAME_API USetMaterialFlagsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	USetMaterialFlagsCommandlet();

	// UCommandlet interface
	virtual int32 Main(const FString& Params) override;
};
