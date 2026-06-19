// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "BT/BTTaskDetectPlayer.h"
#include "FirstGame.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/BaseEnemy.h"

UBTTaskDetectPlayer::UBTTaskDetectPlayer()
{
	NodeName = TEXT("Detect Player");
}

EBTNodeResult::Type UBTTaskDetectPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* Enemy = AIController->GetPawn();
	if (!Enemy) return EBTNodeResult::Failed;

	// Find player pawn
	APawn* PlayerPawn = Enemy->GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!PlayerPawn) return EBTNodeResult::Failed;

	FVector EnemyLocation = Enemy->GetActorLocation();
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	float Distance = FVector::Dist(EnemyLocation, PlayerLocation);

	if (Distance > DetectionRange)
	{
		// Player too far — set aggro to false
		ABaseEnemy* BaseEnemy = Cast<ABaseEnemy>(Enemy);
		if (BaseEnemy) BaseEnemy->SetAggro(false);

		return EBTNodeResult::Failed;
	}

	// Line of sight check
	if (bRequireLineOfSight)
	{
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(Enemy);

		FHitResult HitResult;
		bool bHasLOS = UKismetSystemLibrary::LineTraceSingle(
			Enemy, EnemyLocation, PlayerLocation,
			ETraceTypeQuery::TraceTypeQuery1, false,
			ActorsToIgnore, EDrawDebugTrace::None, HitResult, true
		);

		if (!bHasLOS) return EBTNodeResult::Failed;
	}

	// Player detected — update blackboard and set aggro
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard)
	{
		Blackboard->SetValueAsVector(TEXT("PlayerLocation"), PlayerLocation);
		Blackboard->SetValueAsFloat(TEXT("DistanceToPlayer"), Distance);
	}

	ABaseEnemy* BaseEnemy = Cast<ABaseEnemy>(Enemy);
	if (BaseEnemy) BaseEnemy->SetAggro(true);

	return EBTNodeResult::Succeeded;
}
