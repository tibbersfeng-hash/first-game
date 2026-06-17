// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Subsystems/CombatDataSubsystem.h"

void UCombatDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("CombatDataSubsystem initialized"));
}

UCharacterDataAsset* UCombatDataSubsystem::GetCharacterData(FName CharacterId) const
{
	UCharacterDataAsset* const* Found = CharacterDataMap.Find(CharacterId);
	return Found ? *Found : nullptr;
}

void UCombatDataSubsystem::RegisterCharacterData(FName CharacterId, UCharacterDataAsset* Data)
{
	if (Data)
	{
		CharacterDataMap.Add(CharacterId, Data);
		UE_LOG(LogTemp, Log, TEXT("Registered character data: %s"), *CharacterId.ToString());
	}
}
