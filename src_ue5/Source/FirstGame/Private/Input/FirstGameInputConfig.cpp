// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Input/FirstGameInputConfig.h"
#include "FirstGame.h"

TArray<UInputAction*> UFirstGameInputConfig::GetAllActions() const
{
	TArray<UInputAction*> Actions;

	if (MoveAction) Actions.Add(MoveAction);
	if (LightAttackAction) Actions.Add(LightAttackAction);
	if (HeavyAttackAction) Actions.Add(HeavyAttackAction);
	if (SpecialAction) Actions.Add(SpecialAction);
	if (DodgeAction) Actions.Add(DodgeAction);
	if (JumpAction) Actions.Add(JumpAction);
	if (InteractAction) Actions.Add(InteractAction);
	if (PauseAction) Actions.Add(PauseAction);

	return Actions;
}
