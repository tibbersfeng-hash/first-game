// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Dialogue/DialogueData.h"

UDialogueNodeData::UDialogueNodeData()
{
	DialogueID = NAME_None;
	SpeakerName = FText::FromString(TEXT("未知"));
	DialogueText = FText::FromString(TEXT("..."));
	bIsEnd = false;
}
