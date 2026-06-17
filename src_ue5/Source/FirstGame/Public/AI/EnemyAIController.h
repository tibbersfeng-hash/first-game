// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Enemy AI Controller — Behavior Tree based

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class FIRSTGAME_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	/** Behavior Tree to run */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;

	/** Distance to start chasing player */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float DetectionRange = 600.f;

	/** Distance to start attacking */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AttackRange = 80.f;

	/** Time between attacks */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AttackInterval = 1.5f;

private:
	float LastAttackTime = 0.f;
};
