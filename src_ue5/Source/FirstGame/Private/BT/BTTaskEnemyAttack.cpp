// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "BT/BTTaskEnemyAttack.h"
#include "FirstGame.h"
#include "Characters/BaseEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTaskEnemyAttack::UBTTaskEnemyAttack()
{
	NodeName = TEXT("Enemy Attack");
}

EBTNodeResult::Type UBTTaskEnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(AIController->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	// Check if enemy can attack
	if (Enemy->GetCurrentState() == "Dead" || Enemy->GetCurrentState() == "HitStun")
	{
		return EBTNodeResult::Failed;
	}

	// Check distance to player
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;

	FVector PlayerLocation = Blackboard->GetValueAsVector(TEXT("PlayerLocation"));
	float Distance = FVector::Dist(Enemy->GetActorLocation(), PlayerLocation);

	if (Distance > AttackRange)
	{
		return EBTNodeResult::Failed; // Too far — let chase task handle it
	}

	// Perform attack
	Enemy->PerformAttack();

	return EBTNodeResult::Succeeded;
}
