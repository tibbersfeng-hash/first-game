// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Utils/NPRMaterialUtils.h"

#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/WeakObjectPtrTemplates.h"

#if WITH_EDITORONLY_DATA
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/Material.h"
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

// ─── 描边材质 (暂不实现) ─────────────────────────────────────────────
UMaterial* UNPRMaterialUtils::GetOrCreateOutlineParentMaterial() { return nullptr; }
UMaterialInstanceDynamic* UNPRMaterialUtils::CreateOutlineMaterialInstance(
	UObject* WorldContextObject, const FString& MaterialName,
	const FLinearColor& OutlineColor, float OutlineWidth) { return nullptr; }
AActor* UNPRMaterialUtils::SpawnOutlineAttachment(
	AActor* Monster, const FLinearColor& OutlineColor, float OutlineWidth) { return nullptr; }
