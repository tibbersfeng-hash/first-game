// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Input Setup Helper — creates default InputActions and MappingContext

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputSetupHelper.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Creates default Enhanced Input actions and mapping context.
 * Called during editor initialization to provide default bindings.
 *
 * Keyboard: WASD/Arrows + J/K/L/Shift/Space
 * Gamepad:  Left Stick + X/Y/B/LB/A
 */
UCLASS()
class FIRSTGAME_API UInputSetupHelper : public UObject
{
	GENERATED_BODY()

public:
	/** Create all default InputActions */
	UFUNCTION(BlueprintCallable, Category = "Input Setup")
	static TArray<UInputAction*> CreateDefaultActions(UObject* Outer);

	/** Create the default InputMappingContext */
	UFUNCTION(BlueprintCallable, Category = "Input Setup")
	static UInputMappingContext* CreateDefaultMappingContext(UObject* Outer, const TArray<UInputAction*>& Actions);

	/** Get keyboard bindings description */
	UFUNCTION(BlueprintPure, Category = "Input Setup")
	static FString GetKeyboardBindingsText();

	/** Get gamepad bindings description */
	UFUNCTION(BlueprintPure, Category = "Input Setup")
	static FString GetGamepadBindingsText();
};
