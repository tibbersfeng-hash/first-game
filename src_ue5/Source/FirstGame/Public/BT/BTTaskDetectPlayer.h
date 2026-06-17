// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// BT Task: Detect Player — check if player is within detection range

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskDetectPlayer.generated.h"

UCLASS()
class FIRSTGAME_API UBTTaskDetectPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskDetectPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float DetectionRange = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	bool bRequireLineOfSight = true;
};
