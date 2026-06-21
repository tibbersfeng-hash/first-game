// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Utils/NPRMaterialUtils.h"

#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/WeakObjectPtrTemplates.h"

#if WITH_EDITORONLY_DATA
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionCameraVectorWS.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionPixelNormalWS.h"
#include "Materials/MaterialExpressionPower.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/Material.h"
#include "UObject/SavePackage.h"
#endif

// ─── 静态缓存 ─────────────────────────────────────────────────────────
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
		// 青绿色僵尸，腐烂紫阴影，淡绿 rim
		OutBase   = FLinearColor(0.55f, 0.85f, 0.60f);
		OutShadow = FLinearColor(0.25f, 0.15f, 0.30f);
		OutRim    = FLinearColor(0.80f, 1.00f, 0.70f);
		break;
	case ENPRMonsterPalette::Gingerbread:
		// 暖棕色姜饼人，深红棕阴影，金黄 rim
		OutBase   = FLinearColor(0.85f, 0.55f, 0.30f);
		OutShadow = FLinearColor(0.35f, 0.12f, 0.08f);
		OutRim    = FLinearColor(1.00f, 0.85f, 0.40f);
		break;
	case ENPRMonsterPalette::ShadowNinja:
		// 深紫忍者，品红阴影，电光蓝 rim
		OutBase   = FLinearColor(0.40f, 0.20f, 0.55f);
		OutShadow = FLinearColor(0.15f, 0.05f, 0.20f);
		OutRim    = FLinearColor(0.30f, 0.70f, 1.00f);
		break;
	case ENPRMonsterPalette::ArmoredGum:
		// 银白装甲口香糖，深蓝阴影，亮白 rim
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

// ─── NPR Toon 材质 ────────────────────────────────────────────────────

UMaterial* UNPRMaterialUtils::GetOrCreateNPRParentMaterial()
{
	if (CachedNPRParentMaterial)
	{
		return CachedNPRParentMaterial;
	}

#if WITH_EDITORONLY_DATA
	// 加载已保存的 NPR 父材质资产（shader 已编译）
	const FString ParentPath = TEXT("/Game/Materials/M_NPR_Parent");
	UObject* LoadedObj = StaticLoadObject(UMaterial::StaticClass(), nullptr, *ParentPath);
	UMaterial* Mat = Cast<UMaterial>(LoadedObj);

	if (!Mat)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("GetOrCreateNPRParentMaterial: 未找到资产 %s，降级为运行时创建"),
			*ParentPath);

		// 降级方案：运行时创建 + 保存到磁盘（触发 Shader 编译）
		UPackage* Pkg = CreatePackage(*ParentPath);
		Mat = NewObject<UMaterial>(Pkg, FName(TEXT("M_NPR_Parent")));
		Mat->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
		Mat->SetShadingModel(MSM_Unlit);
		Mat->bUsedWithStaticLighting = false;
		Mat->bUsedWithSkeletalMesh = true;
		BuildNPRMaterialGraph(Mat);
		Mat->PreEditChange(nullptr);
		Mat->PostEditChange();

		// 保存到磁盘
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.Error = GError;
		bool bSaved = UPackage::SavePackage(
			Pkg, Mat,
			*Pkg->GetPathName(),
			SaveArgs);
		UE_LOG(LogTemp, Log,
			TEXT("GetOrCreateNPRParentMaterial: 降级方案保存到 %s = %s"),
			*Pkg->GetPathName(), bSaved ? TEXT("成功") : TEXT("失败"));
	
	}
	else
	{
		UE_LOG(LogTemp, Log,
			TEXT("GetOrCreateNPRParentMaterial: 加载资产 %s 成功"),
			*ParentPath);

		// 强制设置为 Unlit（确保不受灯光影响）
		Mat->SetShadingModel(MSM_Unlit);
		Mat->bUsedWithStaticLighting = false;
		Mat->bUsedWithSkeletalMesh = true;
		UE_LOG(LogTemp, Log,
			TEXT("GetOrCreateNPRParentMaterial: 强制设置 MSM_Unlit"));

		// 如果材质没有表达式（Python 创建的空白材质），构建完整的 NPR 图并保存
		if (Mat->GetExpressionCollection().Expressions.Num() == 0)
		{
			UE_LOG(LogTemp, Log,
				TEXT("GetOrCreateNPRParentMaterial: 构建表达式图..."));
			Mat->bUsedWithSkeletalMesh = true;
			BuildNPRMaterialGraph(Mat);
			Mat->PreEditChange(nullptr);
			Mat->PostEditChange();

			// 保存到磁盘（触发 Shader 编译）
			UPackage* Package = Mat->GetOutermost();
			if (Package)
			{
				Package->MarkPackageDirty();
				FSavePackageArgs SaveArgs;
				SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
				SaveArgs.Error = GError;
				bool bSaved = UPackage::SavePackage(
					Package, Mat,
					*Package->GetPathName(),
					SaveArgs);
				UE_LOG(LogTemp, Log,
					TEXT("GetOrCreateNPRParentMaterial: 保存材质 %s = %s"),
					*Package->GetPathName(), bSaved ? TEXT("成功") : TEXT("失败"));
			}

			UE_LOG(LogTemp, Log,
				TEXT("GetOrCreateNPRParentMaterial: 表达式图构建完成 (Expressions=%d)"),
				Mat->GetExpressionCollection().Expressions.Num());
		}
	}

	CachedNPRParentMaterial = Mat;

	UE_LOG(LogTemp, Log,
		TEXT("NPRMaterialUtils: 创建 NPR 父材质完成 (Expressions=%d)"),
		Mat->GetExpressionCollection().Expressions.Num());

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
	if (!Parent)
	{
		UE_LOG(LogTemp, Error,
			TEXT("CreateNPRMaterialInstance: 无法获取 NPR 父材质"));
		return nullptr;
	}

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Parent, nullptr, FName(*MaterialName));
	if (!MID)
	{
		return nullptr;
	}

	MID->SetVectorParameterValue(FName(TEXT("BaseColor")),       BaseColor);
	MID->SetVectorParameterValue(FName(TEXT("ShadowColor")),     ShadowColor);
	MID->SetVectorParameterValue(FName(TEXT("RimColor")),        RimColor);
	MID->SetScalarParameterValue(FName(TEXT("RimPower")),        RimPower);
	MID->SetScalarParameterValue(FName(TEXT("ShadowThreshold")), 0.3f);
	MID->SetScalarParameterValue(FName(TEXT("ShadowSoftness")),  0.05f);

	UE_LOG(LogTemp, Log,
		TEXT("CreateNPRMaterialInstance: '%s' Base=(%.2f,%.2f,%.2f)"),
		*MaterialName, BaseColor.R, BaseColor.G, BaseColor.B);

	return MID;
}

void UNPRMaterialUtils::ApplyNPRMaterialToMonster(AActor* Enemy, uint8 Palette)
{
	if (!Enemy)
	{
		return;
	}

	// uint8 → 枚举转换，加范围保护
	int32 PaletteIdx = FMath::Clamp<int32>(static_cast<int32>(Palette), 0, 3);
	ENPRMonsterPalette SafePalette = static_cast<ENPRMonsterPalette>(PaletteIdx);

	USkeletalMeshComponent* MeshComp = Enemy->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ApplyNPRMaterialToMonster: %s 无 SkeletalMeshComponent"), *Enemy->GetName());
		return;
	}

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

// ─── Outline 描边材质 ─────────────────────────────────────────────────

UMaterial* UNPRMaterialUtils::GetOrCreateOutlineParentMaterial()
{
	if (CachedOutlineParentMaterial)
	{
		return CachedOutlineParentMaterial;
	}

#if WITH_EDITORONLY_DATA
	UMaterial* Mat = NewObject<UMaterial>(GetTransientPackage());
	Mat->SetFlags(RF_Transactional);

	// 描边材质关键设置
	Mat->SetShadingModel(MSM_Unlit);          // 纯色，不受灯光影响
	Mat->BlendMode     = BLEND_Masked;       // 用 OpacityMask 裁剪正面
	Mat->TwoSided      = true;               // 渲染双面
	Mat->bUsedWithStaticLighting = false;
	Mat->bUsedWithSkeletalMesh = true;

	BuildOutlineMaterialGraph(Mat);

	Mat->PreEditChange(nullptr);
	Mat->PostEditChange();


	CachedOutlineParentMaterial = Mat;

	UE_LOG(LogTemp, Log,
		TEXT("NPRMaterialUtils: 创建 Outline 父材质完成 (Expressions=%d)"),
		Mat->GetExpressionCollection().Expressions.Num());

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
	if (!Parent)
	{
		return nullptr;
	}

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Parent, nullptr, FName(*MaterialName));
	if (!MID)
	{
		return nullptr;
	}

	MID->SetVectorParameterValue(FName(TEXT("OutlineColor")), OutlineColor);
	MID->SetScalarParameterValue(FName(TEXT("OutlineWidth")), OutlineWidth);

	return MID;
}

AActor* UNPRMaterialUtils::SpawnOutlineAttachment(
	AActor* Monster,
	const FLinearColor& OutlineColor,
	float OutlineWidth)
{
	if (!Monster)
	{
		return nullptr;
	}

	UWorld* World = Monster->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	USkeletalMeshComponent* MonsterMesh = Monster->FindComponentByClass<USkeletalMeshComponent>();
	if (!MonsterMesh || !MonsterMesh->SkeletalMesh)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SpawnOutlineAttachment: %s 无有效 SkeletalMesh"), *Monster->GetName());
		return nullptr;
	}

	// ─── 创建描边 Actor（仅承载一个 SkeletalMeshComponent）─────────
	// 直接 new + RegisterComponent 比 SpawnActor 更轻量，
	// 描边是视觉附属物，不需要独立生命周期
	AActor* OutlineActor = NewObject<AActor>(World, AActor::StaticClass(),
		FName(*FString::Printf(TEXT("Outline_%s"), *Monster->GetName())));
	OutlineActor->SetFlags(RF_Transient);

	// 创建描边用的 SkeletalMeshComponent
	USkeletalMeshComponent* OutlineMesh = NewObject<USkeletalMeshComponent>(OutlineActor,
		USkeletalMeshComponent::StaticClass(),
		FName(TEXT("OutlineMeshComp")));

	// 共享相同的 SkeletalMesh 资产
	OutlineMesh->SetSkeletalMesh(MonsterMesh->SkeletalMesh);

	// 复制 AnimClass（ABP 引用），保证动画同步
	UClass* AnimClass = MonsterMesh->GetAnimClass();
	if (AnimClass)
	{
		OutlineMesh->SetAnimInstanceClass(AnimClass);
	}
	else
	{
		// Fallback: 直接复制 AnimInstance 指针（如果运行时已初始化）
		UAnimInstance* MonsterAnimInst = MonsterMesh->GetAnimInstance();
		if (MonsterAnimInst)
		{
			OutlineMesh->SetAnimInstanceClass(MonsterAnimInst->GetClass());
		}
	}

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

	// ─── 组件配置 ───────────────────────────────────────────────────
	OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMesh->bCastDynamicShadow = false;
	OutlineMesh->bCastStaticShadow  = false;
	OutlineMesh->bRenderCustomDepth = false;

	// 注册组件 + 设置 Actor 根节点
	OutlineActor->SetRootComponent(OutlineMesh);
	OutlineActor->AddInstanceComponent(OutlineMesh);
	OutlineMesh->RegisterComponent();

	// Attach 到怪物（保持相对位置不变）
	OutlineMesh->AttachToComponent(Monster->GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// 注册 Actor 到世界
	OutlineActor->RegisterAllComponents();
	OutlineActor->SetActorLocation(Monster->GetActorLocation());

	UE_LOG(LogTemp, Log,
		TEXT("SpawnOutlineAttachment: 为 %s 生成描边 Actor (Width=%.2f)"),
		*Monster->GetName(), OutlineWidth);

	return OutlineActor;
}

// ─── 材质图构建 ───────────────────────────────────────────────────────
//
// NPR Toon Graph (Unlit, EmissiveColor 输出):
//
//   VertexNormalWS ──┐
//                    ├─ DotProduct → NdotL ─┐
//   LightDirection ──┘                      │
//                                            ├─ If(>Threshold)→1, 0 → ShadowMask
//   ShadowThreshold ─────────────────────────┘
//
//   ShadowColor ────┐
//                   ├─ Lerp(Shadow, Base, ShadowMask) → SurfaceColor ─┐
//   BaseColor ──────┘                                                  │
//                                                                       ├─ Add → FinalColor → EmissiveColor
//   VertexNormalWS ──┐                                                 │
//                    ├─ DotProduct → NdotV ─ OneMinus → RimBase ─┐     │
//   CameraVectorWS ──┘                                            │     │
//                                                                  ├─Pow─┘
//   RimPower ─────────────────────────────────────────────────────┤
//                                                                  │
//   RimColor ──── Multiply(RimFactor, RimColor) → RimContrib ─────┘
//
// 参数 (VectorParameter / ScalarParameter):
//   BaseColor, ShadowColor, RimColor (Vector)
//   ShadowThreshold (Scalar, default 0.3)
//   RimPower (Scalar, default 3.0)
//
// 注: LightDirection 用 Constant3Vector 固定 (0.5, 0.3, 0.8)
//     如需动态灯光方向，可改为 VectorParameter（RGB 编码方向）

#if WITH_EDITORONLY_DATA

template <typename ExprClass>
ExprClass* UNPRMaterialUtils::AddExpression(UMaterial* Mat, const TCHAR* DebugName)
{
	ExprClass* Expr = NewObject<ExprClass>(Mat);
	if (DebugName)
	{
		Expr->Desc = DebugName;
	}
	Mat->GetExpressionCollection().AddExpression(Expr);
	return Expr;
}

void UNPRMaterialUtils::BuildNPRMaterialGraph(UMaterial* Mat)
{
	if (!Mat) return;

	UMaterialEditorOnlyData* EO = Mat->GetEditorOnlyData();
	if (!EO)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildNPRMaterialGraph: GetEditorOnlyData() 返回 null"));
		return;
	}

	// ─────────────────────────────────────────────────────────────────
	// 1) 参数节点
	// ─────────────────────────────────────────────────────────────────
	auto* BaseColorParam = AddExpression<UMaterialExpressionVectorParameter>(Mat, TEXT("BaseColor"));
	BaseColorParam->ParameterName = TEXT("BaseColor");
	BaseColorParam->DefaultValue  = FLinearColor(0.8f, 0.6f, 0.4f);

	auto* ShadowColorParam = AddExpression<UMaterialExpressionVectorParameter>(Mat, TEXT("ShadowColor"));
	ShadowColorParam->ParameterName = TEXT("ShadowColor");
	ShadowColorParam->DefaultValue  = FLinearColor(0.3f, 0.15f, 0.2f);

	auto* RimColorParam = AddExpression<UMaterialExpressionVectorParameter>(Mat, TEXT("RimColor"));
	RimColorParam->ParameterName = TEXT("RimColor");
	RimColorParam->DefaultValue  = FLinearColor(1.0f, 0.9f, 0.7f);

	auto* ShadowThresholdParam = AddExpression<UMaterialExpressionScalarParameter>(Mat, TEXT("ShadowThreshold"));
	ShadowThresholdParam->ParameterName = TEXT("ShadowThreshold");
	ShadowThresholdParam->DefaultValue  = 0.3f;

	auto* RimPowerParam = AddExpression<UMaterialExpressionScalarParameter>(Mat, TEXT("RimPower"));
	RimPowerParam->ParameterName = TEXT("RimPower");
	RimPowerParam->DefaultValue  = 3.0f;

	// 主光源方向（固定常数，与场景主光对齐即可）
	// Constant3Vector 的 Constant 字段是 FLinearColor（RGB 编码方向）
	auto* LightDirConst = AddExpression<UMaterialExpressionConstant3Vector>(Mat, TEXT("LightDirection"));
	{
		FVector3f Dir(0.5f, 0.3f, 0.8f);
		Dir.Normalize();
		LightDirConst->Constant = FLinearColor(Dir);
	}

	// ─────────────────────────────────────────────────────────────────
	// 2) 世界空间向量
	// ─────────────────────────────────────────────────────────────────
	auto* VertexNormal = AddExpression<UMaterialExpressionVertexNormalWS>(Mat, TEXT("VertexNormalWS"));
	auto* CameraVec    = AddExpression<UMaterialExpressionCameraVectorWS>(Mat, TEXT("CameraVectorWS"));

	// ─────────────────────────────────────────────────────────────────
	// 3) 阴影遮罩：NdotL > Threshold ? 1 : 0
	// ─────────────────────────────────────────────────────────────────
	auto* NdotL = AddExpression<UMaterialExpressionDotProduct>(Mat, TEXT("NdotL"));
	NdotL->A.Expression = VertexNormal;
	NdotL->B.Expression = LightDirConst;

	auto* ShadowIf = AddExpression<UMaterialExpressionIf>(Mat, TEXT("ShadowStep"));
	ShadowIf->A.Expression = NdotL;
	ShadowIf->B.Expression = ShadowThresholdParam;

	auto* OneConst  = AddExpression<UMaterialExpressionConstant>(Mat, TEXT("1.0"));
	OneConst->R = 1.0f;
	auto* ZeroConst = AddExpression<UMaterialExpressionConstant>(Mat, TEXT("0.0"));
	ZeroConst->R = 0.0f;

	ShadowIf->AGreaterThanB.Expression = OneConst;   // 受光
	ShadowIf->AEqualsB.Expression      = OneConst;
	ShadowIf->ALessThanB.Expression    = ZeroConst;  // 阴影

	// ─────────────────────────────────────────────────────────────────
	// 4) 表面颜色：Lerp(Shadow, Base, ShadowMask)
	// ─────────────────────────────────────────────────────────────────
	auto* ColorLerp = AddExpression<UMaterialExpressionLinearInterpolate>(Mat, TEXT("SurfaceColorLerp"));
	ColorLerp->A.Expression     = ShadowColorParam;  // alpha=0 时显示
	ColorLerp->B.Expression     = BaseColorParam;    // alpha=1 时显示
	ColorLerp->Alpha.Expression = ShadowIf;

	// ─────────────────────────────────────────────────────────────────
	// 5) Rim Light：pow(1 - NdotV, RimPower) * RimColor
	//    注: CameraVectorWS 指向从像素到摄像机的方向
	//        NdotV = dot(N, V) 高 → 正对摄像机，低 → 边缘
	//        1 - NdotV 高 → 边缘，即 rim 区域
	// ─────────────────────────────────────────────────────────────────
	auto* NdotV = AddExpression<UMaterialExpressionDotProduct>(Mat, TEXT("NdotV"));
	NdotV->A.Expression = VertexNormal;
	NdotV->B.Expression = CameraVec;

	auto* OneMinusNdotV = AddExpression<UMaterialExpressionOneMinus>(Mat, TEXT("1 - NdotV"));
	OneMinusNdotV->Input.Expression = NdotV;

	auto* RimPow = AddExpression<UMaterialExpressionPower>(Mat, TEXT("RimPower_Apply"));
	RimPow->Base.Expression     = OneMinusNdotV;
	RimPow->Exponent.Expression = RimPowerParam;

	auto* RimContrib = AddExpression<UMaterialExpressionMultiply>(Mat, TEXT("RimContribution"));
	RimContrib->A.Expression = RimPow;
	RimContrib->B.Expression = RimColorParam;

	// ─────────────────────────────────────────────────────────────────
	// 6) 合成：FinalColor = SurfaceColor + RimContribution
	// ─────────────────────────────────────────────────────────────────
	auto* FinalAdd = AddExpression<UMaterialExpressionAdd>(Mat, TEXT("FinalColor"));
	FinalAdd->A.Expression = ColorLerp;
	FinalAdd->B.Expression = RimContrib;

	// 连接到 EmissiveColor 输出（Unlit 模式下唯一可见的输出）
	EO->EmissiveColor.Expression = FinalAdd;
	EO->EmissiveColor.OutputIndex = 0;

	UE_LOG(LogTemp, Log,
		TEXT("BuildNPRMaterialGraph: 构建完成，节点数 = %d"),
		Mat->GetExpressionCollection().Expressions.Num());
}

// ─── 描边材质图 ───────────────────────────────────────────────────────
//
// Outline (Inverted Hull) Graph:
//
//   VertexNormalWS ──┐
//                    ├─ DotProduct → NdotV ─┐
//   CameraVectorWS ──┘                      │
//                                            ├─ If(NdotV < 0)→1, 0 → OpacityMask
//   Zero ────────────────────────────────────┘
//
//   PixelNormalWS * OutlineWidth → WorldPositionOffset (挤出)
//
//   OutlineColor → EmissiveColor
//
// 渲染效果：
//   - 正面 (NdotV >= 0): OpacityMask=0 → 被裁剪
//   - 背面 (NdotV < 0):  OpacityMask=1 → 可见，并向外挤出形成轮廓

void UNPRMaterialUtils::BuildOutlineMaterialGraph(UMaterial* Mat)
{
	if (!Mat) return;

	UMaterialEditorOnlyData* EO = Mat->GetEditorOnlyData();
	if (!EO)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildOutlineMaterialGraph: GetEditorOnlyData() 返回 null"));
		return;
	}

	// ─────────────────────────────────────────────────────────────────
	// 1) 参数
	// ─────────────────────────────────────────────────────────────────
	auto* OutlineColorParam = AddExpression<UMaterialExpressionVectorParameter>(Mat, TEXT("OutlineColor"));
	OutlineColorParam->ParameterName = TEXT("OutlineColor");
	OutlineColorParam->DefaultValue  = FLinearColor::Black;

	auto* OutlineWidthParam = AddExpression<UMaterialExpressionScalarParameter>(Mat, TEXT("OutlineWidth"));
	OutlineWidthParam->ParameterName = TEXT("OutlineWidth");
	OutlineWidthParam->DefaultValue  = 2.0f;  // cm

	// ─────────────────────────────────────────────────────────────────
	// 2) 正面/背面检测（NdotV 的正负）
	// ─────────────────────────────────────────────────────────────────
	auto* VertexNormal = AddExpression<UMaterialExpressionVertexNormalWS>(Mat, TEXT("VertexNormalWS"));
	auto* CameraVec    = AddExpression<UMaterialExpressionCameraVectorWS>(Mat, TEXT("CameraVectorWS"));

	auto* NdotV = AddExpression<UMaterialExpressionDotProduct>(Mat, TEXT("NdotV"));
	NdotV->A.Expression = VertexNormal;
	NdotV->B.Expression = CameraVec;

	auto* ZeroConst = AddExpression<UMaterialExpressionConstant>(Mat, TEXT("0.0"));
	ZeroConst->R = 0.0f;
	auto* OneConst  = AddExpression<UMaterialExpressionConstant>(Mat, TEXT("1.0"));
	OneConst->R = 1.0f;

	// NdotV < 0 → 背面 → OpacityMask = 1 → 可见
	// NdotV >= 0 → 正面 → OpacityMask = 0 → 裁剪
	auto* BackFaceMask = AddExpression<UMaterialExpressionIf>(Mat, TEXT("BackFaceMask"));
	BackFaceMask->A.Expression = NdotV;
	BackFaceMask->B.Expression = ZeroConst;
	BackFaceMask->AGreaterThanB.Expression = ZeroConst;  // 正面：隐藏
	BackFaceMask->AEqualsB.Expression      = ZeroConst;
	BackFaceMask->ALessThanB.Expression    = OneConst;   // 背面：可见

	// ─────────────────────────────────────────────────────────────────
	// 3) WorldPositionOffset = VertexNormal * OutlineWidth
	//    将顶点沿法线方向挤出（背面法线朝里，所以实际是往里挤？
	//    不对：顶点法线始终朝外，只是背面三角形的缠绕顺序相反。
	//    所有顶点沿法线向外挤，配合 OpacityMask 只保留背面 → 形成轮廓）
	// ─────────────────────────────────────────────────────────────────
	auto* WPO = AddExpression<UMaterialExpressionMultiply>(Mat, TEXT("WPO_Outline"));
	WPO->A.Expression = VertexNormal;
	WPO->B.Expression = OutlineWidthParam;

	// ─────────────────────────────────────────────────────────────────
	// 4) 输出连接
	// ─────────────────────────────────────────────────────────────────
	EO->EmissiveColor.Expression     = OutlineColorParam;  // 纯色描边
	EO->EmissiveColor.OutputIndex    = 0;
	EO->OpacityMask.Expression       = BackFaceMask;
	EO->OpacityMask.OutputIndex      = 0;
	EO->WorldPositionOffset.Expression = WPO;
	EO->WorldPositionOffset.OutputIndex = 0;

	UE_LOG(LogTemp, Log,
		TEXT("BuildOutlineMaterialGraph: 构建完成，节点数 = %d"),
		Mat->GetExpressionCollection().Expressions.Num());
}

#endif // WITH_EDITORONLY_DATA
