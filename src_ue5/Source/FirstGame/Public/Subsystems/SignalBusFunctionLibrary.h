// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// ADR-003: Signal Bus — 全局事件访问辅助

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/SignalBusSubsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SignalBusFunctionLibrary.generated.h"

/**
 * Blueprint-accessible helper to access the SignalBus from anywhere.
 */
UCLASS()
class FIRSTGAME_API USignalBusFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Signal Bus", meta = (WorldContext = "WorldContextObject"))
	static USignalBusSubsystem* GetSignalBus(const UObject* WorldContextObject);
};
