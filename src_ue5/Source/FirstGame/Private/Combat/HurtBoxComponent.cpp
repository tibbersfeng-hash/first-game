// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Combat/HurtBoxComponent.h"

UHurtBoxComponent::UHurtBoxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECR_Overlap); // HitBox channel

	SetBoxExtent(FVector(30.f, 10.f, 50.f));
}

void UHurtBoxComponent::BeginPlay()
{
	Super::BeginPlay();
	OnComponentBeginOverlap.AddDynamic(this, &UHurtBoxComponent::OnDamageOverlapBegin);
}

void UHurtBoxComponent::OnDamageOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bInvincible) return;

	// The HitBox component should have broadcast its own OnHit event
	// This is a secondary notification for the hurt box owner
	UE_LOG(LogTemp, Verbose, TEXT("HurtBox overlap detected"));
}
