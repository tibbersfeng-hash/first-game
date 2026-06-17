// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// BT Task: Chase Player — move toward player within detection range

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskChasePlayer.generated.h"

UCLASS()
class FIRSTGAME_API UBTTaskChasePlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskChasePlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float MoveSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float StopDistance = 60.f;
};
