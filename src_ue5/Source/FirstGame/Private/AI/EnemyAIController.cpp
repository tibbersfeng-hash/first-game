// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "AI/EnemyAIController.h"
#include "FirstGame.h"
#include "Characters/BaseEnemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	bWantsPlayerState = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(InPawn);
	if (!Enemy) return;

	UE_LOG(LogTemp, Log, TEXT("AIController possessing: %s"), *Enemy->GetName());

	// Initialize blackboard keys
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
		BB->SetValueAsFloat(TEXT("DetectionRange"), DetectionRange);
		BB->SetValueAsFloat(TEXT("AttackRange"), AttackRange);
	}

	// Start Behavior Tree
	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(GetPawn());
	if (!Enemy || Enemy->GetCurrentState() == "Dead") return;

	// Update player location in blackboard (for BT tasks)
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (PlayerPawn && GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsVector(TEXT("PlayerLocation"), PlayerPawn->GetActorLocation());
	}
}
