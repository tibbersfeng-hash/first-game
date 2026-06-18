// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Door/DoorActor.h"
#include "FirstGame.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"

ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 碰撞盒 (默认开启, 阻挡玩家)
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(200.f, 50.f, 250.f));  // 宽 4m, 厚 1m, 高 5m
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionObjectType(ECC_WorldStatic);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	SetRootComponent(CollisionBox);

	// 门网格 (用 Cube 代替, 后续可换真实模型)
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(CollisionBox);
	DoorMesh->SetRelativeScale3D(FVector(0.5f, 2.f, 5.f));  // 100cm * (0.5, 2, 5) = (50, 200, 500) cm
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // 碰撞由 CollisionBox 处理

	// 默认红色, 表示"关闭/危险"
	FLinearColor DoorColor(0.7f, 0.2f, 0.2f, 1.f);
	UMaterialInterface* DefaultMat = UMaterial::GetDefaultMaterial(MD_Surface);
	if (DefaultMat)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(DefaultMat, this);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(FName(TEXT("BaseColor")), DoorColor);
			DoorMesh->SetMaterial(0, DynMat);
		}
	}
}

void ADoorActor::BeginPlay()
{
	Super::BeginPlay();

	// 加载默认 Cube mesh
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh && DoorMesh)
	{
		DoorMesh->SetStaticMesh(CubeMesh);
	}

	StartLocation = GetActorLocation();
	TargetLocation = StartLocation + FVector(0.f, 0.f, RaiseHeight);

	UE_LOG(LogTemp, Log, TEXT("DoorActor: 初始化 (位置=%s, 默认关闭)"), *StartLocation.ToString());
}

void ADoorActor::Open()
{
	if (bIsOpen)
	{
		UE_LOG(LogTemp, Warning, TEXT("DoorActor: 已经是开启状态"));
		return;
	}

	bIsOpen = true;
	bIsAnimating = true;
	AnimationProgress = 0.f;

	// 关闭碰撞 (玩家可通过)
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UE_LOG(LogTemp, Log, TEXT("DoorActor: 开门!"));

	OnDoorStateChanged.Broadcast(this);
}

void ADoorActor::Close()
{
	if (!bIsOpen)
	{
		UE_LOG(LogTemp, Warning, TEXT("DoorActor: 已经是关闭状态"));
		return;
	}

	bIsOpen = false;
	bIsAnimating = true;
	AnimationProgress = 0.f;

	// 开启碰撞
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	UE_LOG(LogTemp, Log, TEXT("DoorActor: 关门!"));

	OnDoorStateChanged.Broadcast(this);
}

void ADoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsAnimating) return;

	// 更新动画进度
	AnimationProgress += DeltaTime / OpenDuration;

	if (AnimationProgress >= 1.f)
	{
		// 动画完成
		AnimationProgress = 1.f;
		bIsAnimating = false;
		SetActorLocation(bIsOpen ? TargetLocation : StartLocation);
	}
	else
	{
		// 平滑插值 (ease out cubic)
		float t = AnimationProgress;
		float eased = 1.f - FMath::Pow(1.f - t, 3.f);

		FVector NewLoc = bIsOpen
			? FMath::Lerp(StartLocation, TargetLocation, eased)
			: FMath::Lerp(TargetLocation, StartLocation, eased);
		SetActorLocation(NewLoc);
	}
}
