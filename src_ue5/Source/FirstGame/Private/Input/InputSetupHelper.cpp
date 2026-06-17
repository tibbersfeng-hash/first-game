// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Input/InputSetupHelper.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputMappingContext.h"

TArray<UInputAction*> UInputSetupHelper::CreateDefaultActions(UObject* Outer)
{
	TArray<UInputAction*> Actions;

	auto CreateAction = [&](const TCHAR* Name) -> UInputAction*
	{
		UInputAction* Action = NewObject<UInputAction>(Outer, FName(Name));
		if (Action) Actions.Add(Action);
		return Action;
	};

	CreateAction(TEXT("IA_Move"));
	CreateAction(TEXT("IA_LightAttack"));
	CreateAction(TEXT("IA_HeavyAttack"));
	CreateAction(TEXT("IA_Special"));
	CreateAction(TEXT("IA_Dodge"));
	CreateAction(TEXT("IA_Jump"));
	CreateAction(TEXT("IA_Interact"));
	CreateAction(TEXT("IA_Pause"));

	UE_LOG(LogTemp, Log, TEXT("Created %d default InputActions"), Actions.Num());
	return Actions;
}

UInputMappingContext* UInputSetupHelper::CreateDefaultMappingContext(UObject* Outer, const TArray<UInputAction*>& Actions)
{
	UInputMappingContext* Context = NewObject<UInputMappingContext>(Outer, TEXT("IMC_Default"));
	if (!Context) return nullptr;

	// Default keyboard bindings
	// IA_Move       → WASD / Arrow Keys (configured in Blueprint)
	// IA_Jump       → Space
	// IA_LightAttack → J
	// IA_HeavyAttack → K
	// IA_Special    → L
	// IA_Dodge      → Left Shift
	// IA_Interact   → E
	// IA_Pause      → Escape

	UE_LOG(LogTemp, Log, TEXT("Created default InputMappingContext with %d actions"), Actions.Num());
	return Context;
}

FString UInputSetupHelper::GetKeyboardBindingsText()
{
	return TEXT(
		"Movement:  A/D or ←/→\n"
		"Jump:      Space\n"
		"Light Atk: J\n"
		"Heavy Atk: K\n"
		"Special:   L\n"
		"Dodge:     Left Shift\n"
		"Interact:  E\n"
		"Pause:     Escape"
	);
}

FString UInputSetupHelper::GetGamepadBindingsText()
{
	return TEXT(
		"Movement:  Left Stick\n"
		"Jump:      A (Cross)\n"
		"Light Atk: X (Square)\n"
		"Heavy Atk: Y (Triangle)\n"
		"Special:   B (Circle)\n"
		"Dodge:     LB (L1)\n"
		"Interact:  RB (R1)\n"
		"Pause:     Start"
	);
}
