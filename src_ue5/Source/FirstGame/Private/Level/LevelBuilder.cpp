// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Level/LevelBuilder.h"
#include "FirstGame.h"

#include "Dungeon/DungeonFlow.h"
#include "Dungeon/DungeonRoom.h"
#include "Door/DoorActor.h"
#include "Characters/BaseEnemy.h"
#include "Characters/PlayerCharacter.h"
#include "DataAssets/CharacterDataFactory.h"
#include "DataAssets/CharacterDataAsset.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ─── Tag 常量 ──────────────────────────────────────────────────────────
namespace LevelBuilderTags
{
	static const FName ChildTag = TEXT("LevelBuilder.Child");
	static const FName FloorTag = TEXT("LevelBuilder.Floor");
	static const FName WallTag = TEXT("LevelBuilder.Wall");
	static const FName PillarTag = TEXT("LevelBuilder.Pillar");
	static const FName LightTag = TEXT("LevelBuilder.Light");
}

ALevelBuilder::ALevelBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	// 用一个空 SceneComponent 做根，方便整体变换
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void ALevelBuilder::BeginPlay()
{
	Super::BeginPlay();

	// 确保 TotalRooms 至少为 1
	TotalRooms = FMath::Max(1, TotalRooms);

	LogBuild(FString::Printf(TEXT("=== LevelBuilder: 开始构建关卡 (%d 个房间) ==="), TotalRooms));

	// 1. 场地几何
	BuildArenaGeometry();

	// 2. 灯光
	BuildLighting();

	// 3. 玩家出生点
	SetupPlayerStart();

	// 4. 地牢流程 + 房间
	BuildDungeonFlow();

	// 5. HUD
	SetupHUD();

	// 6. 初始化玩家角色 (赋予 CharacterData, 否则攻击/血量都不工作)
	InitializePlayer();

	LogBuild(FString::Printf(TEXT("=== LevelBuilder: 构建完成, 共生成 %d 个子 Actor ==="), ChildActors.Num()));

	// 自动启动第一个房间
	if (DungeonFlow)
	{
		DungeonFlow->StartDungeon();
	}
}

void ALevelBuilder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupChildren();
	Super::EndPlay(EndPlayReason);
}

// ─── 场地几何 ───────────────────────────────────────────────────────────
void ALevelBuilder::BuildArenaGeometry()
{
	LogBuild(TEXT("BuildArenaGeometry: 开始"));

	for (int32 i = 0; i < TotalRooms; ++i)
	{
		// 房间按 X 轴线性排列
		const float RoomCenterX = i * (RoomExtent.Y + RoomGapDistance);
		const FVector RoomOrigin(RoomCenterX, 0.f, 0.f);
		BuildRoomGeometry(i, RoomOrigin);
	}

	LogBuild(FString::Printf(TEXT("BuildArenaGeometry: 完成, %d 个房间"), TotalRooms));
}

void ALevelBuilder::BuildRoomGeometry(int32 RoomIndex, const FVector& RoomOrigin)
{
	const float HalfW = RoomExtent.X * 0.5f;
	const float HalfL = RoomExtent.Y * 0.5f;
	const float FloorZ = RoomOrigin.Z - RoomExtent.Z * 0.5f + FloorThickness * 0.5f;

	// ─── 地板 ──────────────────────────────────────────────────────
	// 不同房间用不同颜色，便于调试和视觉区分
	const FLinearColor FloorColors[] = {
		FLinearColor(0.35f, 0.40f, 0.45f),  // 深灰蓝
		FLinearColor(0.40f, 0.35f, 0.30f),  // 土黄
		FLinearColor(0.30f, 0.35f, 0.40f),  // 灰蓝
		FLinearColor(0.45f, 0.35f, 0.35f),  // 红褐
		FLinearColor(0.35f, 0.45f, 0.35f),  // 绿灰
	};
	const FLinearColor FloorColor = FloorColors[RoomIndex % UE_ARRAY_COUNT(FloorColors)];

	AActor* Floor = SpawnCube(
		FVector(RoomOrigin.X, RoomOrigin.Y, FloorZ),
		FVector(RoomExtent.X, RoomExtent.Y, FloorThickness),
		FloorColor,
		FName(*FString::Printf(TEXT("Floor_%d"), RoomIndex))
	);
	if (Floor) Floor->Tags.AddUnique(LevelBuilderTags::FloorTag);

	// ─── 四面墙 ────────────────────────────────────────────────────
	const float WallHeight = RoomExtent.Z;
	const float WallCenterZ = RoomOrigin.Z + WallHeight * 0.5f - RoomExtent.Z * 0.5f + FloorThickness;
	const FLinearColor WallColor(0.25f, 0.28f, 0.32f);

	// 后墙 (Y 负向)
	SpawnCube(
		FVector(RoomOrigin.X, RoomOrigin.Y - HalfL - WallThickness * 0.5f, WallCenterZ),
		FVector(RoomExtent.X + WallThickness * 2.f, WallThickness, WallHeight),
		WallColor,
		FName(*FString::Printf(TEXT("Wall_Back_%d"), RoomIndex))
	)->Tags.AddUnique(LevelBuilderTags::WallTag);

	// 前墙 (Y 正向)
	SpawnCube(
		FVector(RoomOrigin.X, RoomOrigin.Y + HalfL + WallThickness * 0.5f, WallCenterZ),
		FVector(RoomExtent.X + WallThickness * 2.f, WallThickness, WallHeight),
		WallColor,
		FName(*FString::Printf(TEXT("Wall_Front_%d"), RoomIndex))
	)->Tags.AddUnique(LevelBuilderTags::WallTag);

	// 左墙 (X 负向) — 第一个房间也留墙，作为起始屏障
	SpawnCube(
		FVector(RoomOrigin.X - HalfW - WallThickness * 0.5f, RoomOrigin.Y, WallCenterZ),
		FVector(WallThickness, RoomExtent.Y, WallHeight),
		WallColor,
		FName(*FString::Printf(TEXT("Wall_Left_%d"), RoomIndex))
	)->Tags.AddUnique(LevelBuilderTags::WallTag);

	// 右墙 (X 正向) — 最后一个房间才封右墙
	if (RoomIndex == TotalRooms - 1)
	{
		SpawnCube(
			FVector(RoomOrigin.X + HalfW + WallThickness * 0.5f, RoomOrigin.Y, WallCenterZ),
			FVector(WallThickness, RoomExtent.Y, WallHeight),
			WallColor,
			FName(*FString::Printf(TEXT("Wall_Right_%d"), RoomIndex))
		)->Tags.AddUnique(LevelBuilderTags::WallTag);
	}

	// ─── 装饰柱（4 个角落） ───────────────────────────────────────
	if (bSpawnPillars)
	{
		const float PillarRadius = 60.f;
		const FLinearColor PillarColor(0.55f, 0.45f, 0.35f);
		const FVector PillarExtents(PillarRadius, PillarRadius, WallHeight * 0.8f);
		const float PillarZ = RoomOrigin.Z - RoomExtent.Z * 0.5f + FloorThickness + WallHeight * 0.4f;

		const FVector2D PillarOffsets[] = {
			FVector2D(-HalfW * 0.7f, -HalfL * 0.7f),
			FVector2D(-HalfW * 0.7f,  HalfL * 0.7f),
			FVector2D( HalfW * 0.7f, -HalfL * 0.7f),
			FVector2D( HalfW * 0.7f,  HalfL * 0.7f),
		};

		for (int32 P = 0; P < UE_ARRAY_COUNT(PillarOffsets); ++P)
		{
			AActor* Pillar = SpawnCube(
				FVector(RoomOrigin.X + PillarOffsets[P].X, RoomOrigin.Y + PillarOffsets[P].Y, PillarZ),
				PillarExtents,
				PillarColor,
				FName(*FString::Printf(TEXT("Pillar_%d_%d"), RoomIndex, P))
			);
			if (Pillar) Pillar->Tags.AddUnique(LevelBuilderTags::PillarTag);
		}
	}
}

// ─── 灯光 ───────────────────────────────────────────────────────────────
void ALevelBuilder::BuildLighting()
{
	LogBuild(TEXT("BuildLighting: 开始"));

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// ─── 方向光（太阳）───────────────────────────────────────────
	ADirectionalLight* DirLight = World->SpawnActor<ADirectionalLight>(
		ADirectionalLight::StaticClass(),
		FVector::ZeroVector,
		DirectionalLightRotation,
		SpawnParams
	);
	if (DirLight)
	{
		if (ULightComponent* LightComp = DirLight->GetLightComponent())
		{
			LightComp->SetIntensity(DirectionalLightIntensity);
			LightComp->SetLightColor(DirectionalLightColor);
		}
		DirLight->SetActorRotation(DirectionalLightRotation);
		DirLight->Tags.AddUnique(LevelBuilderTags::ChildTag);
		DirLight->Tags.AddUnique(LevelBuilderTags::LightTag);
		ChildActors.Add(DirLight);
	}

	// ─── 每个房间一个点光 ────────────────────────────────────────
	for (int32 i = 0; i < TotalRooms; ++i)
	{
		const float RoomCenterX = i * (RoomExtent.Y + RoomGapDistance);
		const FVector LightPos(RoomCenterX, 0.f, RoomExtent.Z * 0.8f);

		APointLight* PointLight = World->SpawnActor<APointLight>(
			APointLight::StaticClass(),
			LightPos,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (PointLight)
		{
			if (UPointLightComponent* PLC = Cast<UPointLightComponent>(PointLight->GetLightComponent()))
			{
				PLC->SetIntensity(3000.f);
				PLC->SetAttenuationRadius(RoomExtent.Y * 0.8f);
				PLC->SetLightColor(FLinearColor(1.f, 0.9f, 0.75f));
				PLC->SetCastShadows(false);
			}
			PointLight->Tags.AddUnique(LevelBuilderTags::ChildTag);
			PointLight->Tags.AddUnique(LevelBuilderTags::LightTag);
			ChildActors.Add(PointLight);
		}
	}

	LogBuild(FString::Printf(TEXT("BuildLighting: 完成, 共 %d 个光源"), ChildActors.Num()));
}

// ─── 玩家出生点 ─────────────────────────────────────────────────────────
void ALevelBuilder::SetupPlayerStart()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 先查找场景中是否已有 PlayerStart（避免重复）
	AActor* ExistingStart = UGameplayStatics::GetActorOfClass(World, APlayerStart::StaticClass());
	if (ExistingStart)
	{
		LogBuild(TEXT("SetupPlayerStart: 发现已有 PlayerStart, 复用"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 第一个房间中心
	const FVector StartLocation(0.f, 0.f, FloorThickness * 0.5f + 100.f);

	APlayerStart* PlayerStart = World->SpawnActor<APlayerStart>(
		APlayerStart::StaticClass(),
		StartLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);
	if (PlayerStart)
	{
		PlayerStart->Tags.AddUnique(LevelBuilderTags::ChildTag);
		ChildActors.Add(PlayerStart);
		LogBuild(FString::Printf(TEXT("SetupPlayerStart: Spawn 于 %s"), *StartLocation.ToString()));
	}
}

// ─── 地牢流程 ───────────────────────────────────────────────────────────
void ALevelBuilder::BuildDungeonFlow()
{
	LogBuild(TEXT("BuildDungeonFlow: 开始"));

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 1. Spawn DungeonFlow
	DungeonFlow = World->SpawnActor<ADungeonFlow>(
		ADungeonFlow::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);
	if (DungeonFlow)
	{
		DungeonFlow->Tags.AddUnique(LevelBuilderTags::ChildTag);
		ChildActors.Add(DungeonFlow);
	}

	// 2. Spawn N 个 DungeonRoom
	for (int32 i = 0; i < TotalRooms; ++i)
	{
		const float RoomCenterX = i * (RoomExtent.Y + RoomGapDistance);
		const FVector RoomOrigin(RoomCenterX, 0.f, 0.f);

		ADungeonRoom* Room = BuildDungeonRoom(i, RoomOrigin);
		if (Room)
		{
			SpawnedRooms.Add(Room);
		}
	}

	// 3. 在房间之间生成门 (最后一个房间后不需要门)
	for (int32 i = 0; i < TotalRooms - 1; ++i)
	{
		if (!SpawnedRooms.IsValidIndex(i)) continue;

		// 门位置: 当前房间和下一房间之间的中点
		const float CurrentRoomX = i * (RoomExtent.Y + RoomGapDistance);
		const float NextRoomX = (i + 1) * (RoomExtent.Y + RoomGapDistance);
		const float DoorX = (CurrentRoomX + NextRoomX) * 0.5f;
		const FVector DoorLocation(DoorX, 0.f, FloorThickness + 250.f);  // 门底部在地面上

		ADoorActor* Door = World->SpawnActor<ADoorActor>(
			ADoorActor::StaticClass(),
			DoorLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Door)
		{
			Door->Tags.AddUnique(LevelBuilderTags::ChildTag);
			ChildActors.Add(Door);
			SpawnedDoors.Add(Door);

			// 门属于前一个房间的出口
			SpawnedRooms[i]->ExitDoor = Door;

			LogBuild(FString::Printf(
				TEXT("BuildDungeonFlow: 在房间 %d 和 %d 之间生成门 (位置=%s)"),
				i, i + 1, *DoorLocation.ToString()));
		}
	}

	// 3. 初始化 DungeonFlow（喂入所有房间）
	if (DungeonFlow && SpawnedRooms.Num() > 0)
	{
		DungeonFlow->InitializeDungeon(SpawnedRooms);
		LogBuild(FString::Printf(TEXT("BuildDungeonFlow: 初始化完成, %d 个房间"), SpawnedRooms.Num()));
	}
}

ADungeonRoom* ALevelBuilder::BuildDungeonRoom(int32 RoomIndex, const FVector& RoomOrigin)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 房间 Actor 放在房间中心稍高处（避免和地板穿插）
	const FVector RoomActorLoc(RoomOrigin.X, RoomOrigin.Y, FloorThickness + 10.f);

	ADungeonRoom* Room = World->SpawnActor<ADungeonRoom>(
		ADungeonRoom::StaticClass(),
		RoomActorLoc,
		FRotator::ZeroRotator,
		SpawnParams
	);
	if (!Room) return nullptr;

	Room->Tags.AddUnique(LevelBuilderTags::ChildTag);
	ChildActors.Add(Room);

	// 配置房间
	Room->RoomIndex = RoomIndex;
	Room->RoomType = (RoomIndex == TotalRooms - 1 && bBossOnFinalRoom) ? ERoomType::Boss : ERoomType::Combat;

	// 给房间填充敌人 Class
	if (DefaultEnemyClass)
	{
		int32 EnemyCount = BaseEnemyCountPerRoom;
		if (Room->RoomType == ERoomType::Boss)
		{
			EnemyCount *= 2;  // Boss 房敌人加倍
			// Boss 房所有敌人一波出, 更有压迫感
			Room->EnemiesPerWave = EnemyCount;
		}
		else
		{
			// 普通房: 默认 3 个一波, DungeonRoom 自动切分
			Room->EnemiesPerWave = FMath::Min(3, EnemyCount);
		}
		for (int32 E = 0; E < EnemyCount; ++E)
		{
			Room->EnemyClasses.Add(DefaultEnemyClass);
		}
	}

	LogBuild(FString::Printf(
		TEXT("BuildDungeonRoom[%d]: 类型=%s, 敌人=%d, 每波=%d, 位置=%s"),
		RoomIndex,
		Room->RoomType == ERoomType::Boss ? TEXT("Boss") : TEXT("Combat"),
		Room->EnemyClasses.Num(),
		Room->EnemiesPerWave,
		*RoomActorLoc.ToString()
	));

	return Room;
}

// ─── HUD ────────────────────────────────────────────────────────────────
void ALevelBuilder::SetupHUD()
{
	if (!HUDWidgetClass)
	{
		LogBuild(TEXT("SetupHUD: 未配置 HUDWidgetClass, 跳过"));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		LogBuild(TEXT("SetupHUD: 未找到 PlayerController, 跳过"));
		return;
	}

	UUserWidget* HUD = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
	if (HUD)
	{
		HUD->AddToViewport(0);
		LogBuild(TEXT("SetupHUD: HUD Widget 已添加到 Viewport"));
	}
}

// ─── 玩家角色初始化 ────────────────────────────────────────────────────
void ALevelBuilder::InitializePlayer()
{
	LogBuild(TEXT("InitializePlayer: 开始"));

	// 找到场景中的 PlayerCharacter (由 GameMode 在 PlayerStart 处生成)
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(
		UGameplayStatics::GetPlayerPawn(this, 0)
	);

	if (!Player)
	{
		// 没找到 PlayerPawn, 尝试搜索所有 Actor
		for (TActorIterator<APlayerCharacter> It(World); It; ++It)
		{
			Player = *It;
			break;
		}
	}

	if (!Player)
	{
		LogBuild(TEXT("InitializePlayer: 未找到 PlayerCharacter, 跳过"));
		return;
	}

	// 用 CharacterDataFactory 创建角色数据
	UCharacterDataAsset* DataAsset = nullptr;
	if (DefaultCharacterId == FName(TEXT("Huikong")))
	{
		DataAsset = UCharacterDataFactory::CreateHuikongData(Player);
	}
	else if (DefaultCharacterId == FName(TEXT("Tangtang")))
	{
		DataAsset = UCharacterDataFactory::CreateTangtangData(Player);
	}
	else if (DefaultCharacterId == FName(TEXT("Kiguemaru")))
	{
		DataAsset = UCharacterDataFactory::CreateKiguemaruData(Player);
	}
	else
	{
		// 默认用 Huikong
		LogBuild(FString::Printf(
			TEXT("InitializePlayer: 未知角色 ID '%s', 使用 Huikong"),
			*DefaultCharacterId.ToString()));
		DataAsset = UCharacterDataFactory::CreateHuikongData(Player);
	}

	if (!DataAsset)
	{
		LogBuild(TEXT("InitializePlayer: CharacterDataFactory 创建失败"));
		return;
	}

	// 初始化角色 (设置血量/能量/移动速度/攻击数据)
	Player->InitializeCharacter(DataAsset);

	LogBuild(FString::Printf(
		TEXT("InitializePlayer: 角色 '%s' 已初始化 (HP:%.0f, Speed:%.0f)"),
		*DefaultCharacterId.ToString(),
		DataAsset->MaxHealth,
		DataAsset->MoveSpeed));
}

// ─── 工具方法 ───────────────────────────────────────────────────────────
AActor* ALevelBuilder::SpawnCube(const FVector& Center, const FVector& Extent,
                                  const FLinearColor& Color, FName TagSuffix)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	// 加载引擎默认 Cube Mesh
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	if (!CubeMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("LevelBuilder: 无法加载 /Engine/BasicShapes/Cube"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(
		AActor::StaticClass(),
		Center,
		FRotator::ZeroRotator,
		SpawnParams
	);
	if (!NewActor) return nullptr;

	// 创建 StaticMeshComponent
	UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(NewActor, TEXT("MeshComp"));
	MeshComp->SetStaticMesh(CubeMesh);

	// 缩放: UE Cube 默认 100x100x100 cm, 按 Extent 缩放
	const FVector Scale(Extent.X / 100.f, Extent.Y / 100.f, Extent.Z / 100.f);
	MeshComp->SetWorldScale3D(Scale);

	// 材质: 创建动态材质实例，染成指定颜色
	// 用引擎默认材质 (DefaultMaterial) 作为 parent
	UMaterialInterface* DefaultMat = UMaterial::GetDefaultMaterial(MD_Surface);
	if (DefaultMat)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(DefaultMat, NewActor);
		if (DynMat)
		{
			// UE DefaultMaterial 有 "BaseColor" 参数
			DynMat->SetVectorParameterValue(FName(TEXT("BaseColor")), Color);
			MeshComp->SetMaterial(0, DynMat);
		}
	}

	// 物理: Static (不动)
	MeshComp->SetMobility(EComponentMobility::Static);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetGenerateOverlapEvents(false);

	NewActor->SetRootComponent(MeshComp);
	NewActor->Tags.AddUnique(LevelBuilderTags::ChildTag);

	// 给 Tag 加上语义后缀，便于调试时搜索
	if (!TagSuffix.IsNone())
	{
		NewActor->Tags.AddUnique(TagSuffix);
	}

	ChildActors.Add(NewActor);
	return NewActor;
}

void ALevelBuilder::CleanupChildren()
{
	LogBuild(FString::Printf(TEXT("CleanupChildren: 清理 %d 个子 Actor"), ChildActors.Num()));

	for (AActor* Child : ChildActors)
	{
		if (Child && IsValid(Child))
		{
			Child->Destroy();
		}
	}
	ChildActors.Empty();
	SpawnedRooms.Empty();
	SpawnedDoors.Empty();
	DungeonFlow = nullptr;
}

void ALevelBuilder::LogBuild(const FString& Message) const
{
	if (bDebugMode)
	{
		UE_LOG(LogTemp, Log, TEXT("[LevelBuilder] %s"), *Message);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("[LevelBuilder] %s"), *Message);
	}
}
