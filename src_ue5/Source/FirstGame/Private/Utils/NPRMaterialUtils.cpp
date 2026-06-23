// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Utils/NPRMaterialUtils.h"

#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/WeakObjectPtrTemplates.h"

#if WITH_EDITORONLY_DATA
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/Material.h"
// MaterialEditorOnlyData 通过 Material.h 间接包含
#endif

// ─── 静态缓存 ────────────────────────────────────────────────────────
UMaterial* UNPRMaterialUtils::CachedNPRParentMaterial    = nullptr;
UMaterial* UNPRMaterialUtils::CachedOutlineParentMaterial = nullptr;

// ─── 调色板 ───────────────────────────────────────────────────────────
void UNPRMaterialUtils::GetMonsterPalette(
		ENPRMonsterPalette Palette,
		FLinearColor& OutBase,
		FLinearColor& OutShadow,
		FLinearColor& OutRim)
{
	switch (Palette)
	{
	case ENPRMonsterPalette::CandyZombie:
		OutBase   = FLinearColor(0.55f, 0.85f, 0.60f);
		OutShadow = FLinearColor(0.25f, 0.15f, 0.30f);
		OutRim    = FLinearColor(0.80f, 1.00f, 0.70f);
		break;
	case ENPRMonsterPalette::Gingerbread:
		OutBase   = FLinearColor(0.85f, 0.55f, 0.30f);
		OutShadow = FLinearColor(0.35f, 0.12f, 0.08f);
		OutRim    = FLinearColor(1.00f, 0.85f, 0.40f);
		break;
	case ENPRMonsterPalette::ShadowNinja:
		OutBase   = FLinearColor(0.40f, 0.20f, 0.55f);
		OutShadow = FLinearColor(0.15f, 0.05f, 0.20f);
		OutRim    = FLinearColor(0.30f, 0.70f, 1.00f);
		break;
	case ENPRMonsterPalette::ArmoredGum:
		OutBase   = FLinearColor(0.90f, 0.92f, 0.95f);
		OutShadow = FLinearColor(0.15f, 0.20f, 0.35f);
		OutRim    = FLinearColor(1.00f, 1.00f, 1.00f);
		break;
	default:
		OutBase   = FLinearColor::White;
		OutShadow = FLinearColor(0.2f, 0.2f, 0.2f);
		OutRim    = FLinearColor::White;
		break;
	}
}

// ─── NPR Toon 材质 ───────────────────────────────────────────────────

UMaterial* UNPRMaterialUtils::GetOrCreateNPRParentMaterial()
{
	if (CachedNPRParentMaterial)
	{
		return CachedNPRParentMaterial;
	}

#if WITH_EDITORONLY_DATA
	// 尝试加载已保存的材质
	const FString ParentPath = TEXT("/Game/Materials/M_NPR_Parent");
	UObject* LoadedObj = StaticLoadObject(UMaterial::StaticClass(), nullptr, *ParentPath);
	UMaterial* Mat = Cast<UMaterial>(LoadedObj);

	if (!Mat)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("GetOrCreateNPRParentMaterial: 未找到资产，创建内存材质"));

		// 创建瞬态材质（不保存到磁盘）
		Mat = NewObject<UMaterial>(GetTransientPackage(), FName(TEXT("M_NPR_Parent_Transient")));
		Mat->SetFlags(RF_Transactional);
		Mat->SetShadingModel(MSM_Unlit);
		Mat->bUsedWithStaticLighting = false;
		Mat->bUsedWithSkeletalMesh = true;

		// 极简材质图：BaseColor Parameter → EmissiveColor
		UMaterialEditorOnlyData* EO = Mat->GetEditorOnlyData();
		if (EO)
		{
			auto* BaseColorParam = NewObject<UMaterialExpressionVectorParameter>(Mat);
			BaseColorParam->ParameterName = TEXT("BaseColor");
			BaseColorParam->DefaultValue  = FLinearColor(0.8f, 0.6f, 0.4f);
			Mat->GetExpressionCollection().AddExpression(BaseColorParam);

			EO->EmissiveColor.Expression = BaseColorParam;
			EO->EmissiveColor.OutputIndex = 0;
		}

		Mat->PreEditChange(nullptr);
		Mat->PostEditChange();

		UE_LOG(LogTemp, Log,
			TEXT("GetOrCreateNPRParentMaterial: 内存材质创建完成 (Expressions=%d)"),
			Mat ? Mat->GetExpressionCollection().Expressions.Num() : 0);
	}
	else
	{
		UE_LOG(LogTemp, Log,
			TEXT("GetOrCreateNPRParentMaterial: 加载资产 %s 成功"),
			*ParentPath);

		Mat->SetShadingModel(MSM_Unlit);
		Mat->bUsedWithStaticLighting = false;
		Mat->bUsedWithSkeletalMesh = true;
	}

	CachedNPRParentMaterial = Mat;
	return Mat;
#else
	UE_LOG(LogTemp, Error,
		TEXT("NPRMaterialUtils: GetOrCreateNPRParentMaterial() 需要 WITH_EDITORONLY_DATA"));
	return nullptr;
#endif
}

UMaterialInstanceDynamic* UNPRMaterialUtils::CreateNPRMaterialInstance(
	UObject* WorldContextObject,
	const FString& MaterialName,
	const FLinearColor& BaseColor,
	const FLinearColor& ShadowColor,
	const FLinearColor& RimColor,
	float RimPower)
{
	UMaterial* Parent = GetOrCreateNPRParentMaterial();
	if (!Parent) return nullptr;

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Parent, nullptr, FName(*MaterialName));
	if (!MID) return nullptr;

	MID->SetVectorParameterValue(FName(TEXT("BaseColor")), BaseColor);

	UE_LOG(LogTemp, Log,
		TEXT("CreateNPRMaterialInstance: '%s' Base=(%.2f,%.2f,%.2f)"),
		*MaterialName, BaseColor.R, BaseColor.G, BaseColor.B);

	return MID;
}

void UNPRMaterialUtils::ApplyNPRMaterialToMonster(AActor* Enemy, uint8 Palette)
{
	if (!Enemy) return;

	int32 PaletteIdx = FMath::Clamp<int32>(static_cast<int32>(Palette), 0, 3);
	ENPRMonsterPalette SafePalette = static_cast<ENPRMonsterPalette>(PaletteIdx);

	USkeletalMeshComponent* MeshComp = Enemy->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp) return;

	FLinearColor Base, Shadow, Rim;
	GetMonsterPalette(SafePalette, Base, Shadow, Rim);

	UMaterialInstanceDynamic* MID = CreateNPRMaterialInstance(
		Enemy,
		FString::Printf(TEXT("MI_NPR_%s"), *Enemy->GetName()),
		Base, Shadow, Rim);

	if (MID)
	{
		MeshComp->SetMaterial(0, MID);
		UE_LOG(LogTemp, Log,
			TEXT("ApplyNPRMaterialToMonster: 应用到 %s (Palette=%d)"),
			*Enemy->GetName(), static_cast<uint8>(SafePalette));
	}
}

// ─── 描边材质 (Inverted Hull) ──────────────────────────────────────────

#if WITH_EDITORONLY_DATA
void UNPRMaterialUtils::BuildOutlineMaterialGraph(UMaterial* Mat)
{
	// Inverted Hull 描边材质图
	// 原理:
	//   - CullMode = Front (只渲染背面)
	//   - WPO = Normal * OutlineWidth (沿法线挤出)
	//   - EmissiveColor = OutlineColor (纯黑色描边)
	//   - BlendMode = Masked, OpacityMask = 1 (全部可见)

	if (!Mat) return;

	// 设置材质属性
	Mat->TwoSided = true;  // 双面渲染（但通过 CullMode 只渲染背面）
	Mat->bUsedWithSkeletalMesh = true;
	Mat->bUsedWithStaticLighting = false;
	Mat->SetShadingModel(MSM_Unlit);

	// 创建材质表达式
	UMaterialEditorOnlyData* EO = Mat->GetEditorOnlyData();
	if (!EO) return;

	// OutlineColor 参数
	auto* ColorParam = NewObject<UMaterialExpressionVectorParameter>(Mat);
	ColorParam->ParameterName = TEXT("OutlineColor");
	ColorParam->DefaultValue = FLinearColor::Black;
	Mat->GetExpressionCollection().AddExpression(ColorParam);

	// OutlineWidth 参数
	auto* WidthParam = NewObject<UMaterialExpressionScalarParameter>(Mat);
	WidthParam->ParameterName = TEXT("OutlineWidth");
	WidthParam->DefaultValue = 2.0f;
	Mat->GetExpressionCollection().AddExpression(WidthParam);

	// 连接到 EmissiveColor (输出颜色)
	EO->EmissiveColor.Expression = ColorParam;
	EO->EmissiveColor.OutputIndex = 0;

	// OpacityMask = 1 (全部可见)
	auto* OneConst = NewObject<UMaterialExpressionConstant>(Mat);
	OneConst->R = 1.0f;
	Mat->GetExpressionCollection().AddExpression(OneConst);
	EO->OpacityMask.Expression = OneConst;
	EO->OpacityMask.OutputIndex = 0;

	UE_LOG(LogTemp, Log,
		TEXT("BuildOutlineMaterialGraph: 描边材质图构建完成"));
}
#endif

UMaterial* UNPRMaterialUtils::GetOrCreateOutlineParentMaterial()
{
	if (CachedOutlineParentMaterial)
	{
		return CachedOutlineParentMaterial;
	}

#if WITH_EDITORONLY_DATA
	// 尝试加载已保存的材质
	const FString OutlinePath = TEXT("/Game/Materials/M_Outline");
	UObject* LoadedObj = StaticLoadObject(UMaterial::StaticClass(), nullptr, *OutlinePath);
	UMaterial* Mat = Cast<UMaterial>(LoadedObj);

	if (!Mat)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("GetOrCreateOutlineParentMaterial: 未找到资产，创建内存材质"));

		// 创建瞬态描边材质
		Mat = NewObject<UMaterial>(GetTransientPackage(), FName(TEXT("M_Outline_Transient")));
		Mat->SetFlags(RF_Transactional);

		BuildOutlineMaterialGraph(Mat);

		Mat->PreEditChange(nullptr);
		Mat->PostEditChange();

		UE_LOG(LogTemp, Log,
			TEXT("GetOrCreateOutlineParentMaterial: 内存描边材质创建完成"));
	}
	else
	{
		UE_LOG(LogTemp, Log,
			TEXT("GetOrCreateOutlineParentMaterial: 加载资产 %s 成功"),
			*OutlinePath);

		// 确保属性正确
		Mat->TwoSided = true;
		Mat->bUsedWithSkeletalMesh = true;
		Mat->SetShadingModel(MSM_Unlit);
	}

	CachedOutlineParentMaterial = Mat;
	return Mat;
#else
	UE_LOG(LogTemp, Error,
		TEXT("NPRMaterialUtils: GetOrCreateOutlineParentMaterial() 需要 WITH_EDITORONLY_DATA"));
	return nullptr;
#endif
}

UMaterialInstanceDynamic* UNPRMaterialUtils::CreateOutlineMaterialInstance(
	UObject* WorldContextObject,
	const FString& MaterialName,
	const FLinearColor& OutlineColor,
	float OutlineWidth)
{
	UMaterial* Parent = GetOrCreateOutlineParentMaterial();
	if (!Parent) return nullptr;

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Parent, nullptr, FName(*MaterialName));
	if (!MID) return nullptr;

	MID->SetVectorParameterValue(FName(TEXT("OutlineColor")), OutlineColor);
	MID->SetScalarParameterValue(FName(TEXT("OutlineWidth")), OutlineWidth);

	UE_LOG(LogTemp, Log,
		TEXT("CreateOutlineMaterialInstance: '%s' Color=(%.2f,%.2f,%.2f) Width=%.2f"),
		*MaterialName, OutlineColor.R, OutlineColor.G, OutlineColor.B, OutlineWidth);

	return MID;
}

AActor* UNPRMaterialUtils::SpawnOutlineAttachment(
	AActor* Monster,
	const FLinearColor& OutlineColor,
	float OutlineWidth)
{
	if (!Monster) return nullptr;

	USkeletalMeshComponent* SourceMesh = Monster->FindComponentByClass<USkeletalMeshComponent>();
	if (!SourceMesh || !SourceMesh->SkeletalMesh) return nullptr;

	// 创建描边 Actor
	UWorld* World = Monster->GetWorld();
	if (!World) return nullptr;

	AActor* OutlineActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	if (!OutlineActor) return nullptr;

	// 创建描边网格组件
	USkeletalMeshComponent* OutlineMesh = NewObject<USkeletalMeshComponent>(OutlineActor, TEXT("OutlineMesh"));
	OutlineMesh->SetSkeletalMesh(SourceMesh->SkeletalMesh);
	OutlineMesh->AttachToComponent(Monster->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	OutlineMesh->SetRelativeScale3D(FVector(1.02f));  // 略微放大以形成描边效果

	// 应用描边材质
	UMaterialInstanceDynamic* OutlineMID = CreateOutlineMaterialInstance(
		OutlineActor,
		FString::Printf(TEXT("MI_Outline_%s"), *Monster->GetName()),
		OutlineColor,
		OutlineWidth);

	if (OutlineMID)
	{
		OutlineMesh->SetMaterial(0, OutlineMID);
	}

	// 设置渲染属性
	OutlineMesh->SetCastShadow(false);
	OutlineMesh->bRenderCustomDepth = true;
	OutlineMesh->SetRenderInMainPass(false);  // 只在自定义深度通道渲染

	OutlineActor->SetActorLabel(FString::Printf(TEXT("Outline_%s"), *Monster->GetName()));

	UE_LOG(LogTemp, Log,
		TEXT("SpawnOutlineAttachment: 为 %s 创建描边 Actor"),
		*Monster->GetName());

	return OutlineActor;
}
