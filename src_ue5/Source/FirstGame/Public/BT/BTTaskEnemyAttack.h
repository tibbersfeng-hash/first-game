// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// BT Task: Enemy Attack — perform attack when in range

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskEnemyAttack.generated.h"

UCLASS()
class FIRSTGAME_API UBTTaskEnemyAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskEnemyAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float AttackRange = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float AttackCooldown = 1.5f;
};
