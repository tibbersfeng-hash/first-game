// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "BT/BTTaskChasePlayer.h"
#include "FirstGame.h"
#include "Characters/BaseEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTaskChasePlayer::UBTTaskChasePlayer()
{
	NodeName = TEXT("Chase Player");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTaskChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* Enemy = AIController->GetPawn();
	if (!Enemy) return EBTNodeResult::Failed;

	// Get player location from blackboard
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;

	FVector PlayerLocation = Blackboard->GetValueAsVector(TEXT("PlayerLocation"));
	FVector EnemyLocation = Enemy->GetActorLocation();

	float Distance = FVector::Dist(PlayerLocation, EnemyLocation);

	if (Distance <= StopDistance)
	{
		// Close enough — let attack task take over
		return EBTNodeResult::Succeeded;
	}

	// Move toward player (2D: only X axis)
	FVector Direction = (PlayerLocation - EnemyLocation).GetSafeNormal();
	Direction.Y = 0.f; // Lock to XZ plane

	if (UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(Enemy->GetMovementComponent()))
	{
		MoveComp->MaxWalkSpeed = MoveSpeed;
		Enemy->AddMovementInput(Direction);
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTaskChasePlayer::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Stop movement when task is aborted
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}
	return EBTNodeResult::Aborted;
}
