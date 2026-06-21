// Copyright 2026 格斗萌主 Team. All Rights Reserved.

// NPRMaterialUtils — C++ NPR 材质工具库
// 生成赛璐璐风格（2-tone 阴影 + Rim Light）及描边（Inverted Hull）材质
//
// 设计思路：
//   - ParentMaterial 在 Editor 环境下一次性构建（含完整 Expression 节点图），缓存到静态指针
//   - 运行时通过 UMaterialInstanceDynamic::Create() 派生 MID，仅调参不重建图
//   - 描边材质使用 TwoSided + WPO 挤出背面实现 Inverted Hull 效果

#pragma once

#include "CoreMinimal.h"
#include "Materials/Material.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NPRMaterialUtils.generated.h"

class UMaterialInstanceDynamic;

/**
 * 4 只糖果地牢怪物的 NPR 配色方案
 * 在 DataAsset 化之前先用枚举索引
 */
UENUM(BlueprintType)
enum class ENPRMonsterPalette : uint8
{
	CandyZombie   = 0,  // 青绿 + 深紫阴影
	Gingerbread   = 1,  // 暖棕 + 深红阴影
	ShadowNinja   = 2,  // 深紫 + 品红阴影
	ArmoredGum    = 3,  // 银白 + 深蓝阴影
};

/**
 * NPR 材质工具类（UMG / BP 可调用）
 *
 * 提供两种材质：
 *   1. NPR Toon — 2-tone 阴影 + Rim Light (Unlit, 自包含)
 *   2. Outline  — Inverted Hull 描边 (Masked, TwoSided, WPO 挤出)
 */
UCLASS()
class FIRSTGAME_API UNPRMaterialUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ─── NPR Toon 材质 ──────────────────────────────────────────────

	/**
	 * 创建 NPR 卡通材质实例（运行时调用）
	 * @param WorldContextObject 提供 UWorld 上下文
	 * @param MaterialName       MID 名称（调试用）
	 * @param BaseColor          受光面颜色
	 * @param ShadowColor        阴影面颜色
	 * @param RimColor           Rim 光颜色
	 * @param RimPower           Rim 光衰减指数（越大边缘越锐）
	 * @return 创建的 MID，失败返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "NPR|Material",
		meta = (WorldContext = "WorldContextObject"))
	static UMaterialInstanceDynamic* CreateNPRMaterialInstance(
		UObject* WorldContextObject,
		const FString& MaterialName,
		const FLinearColor& BaseColor,
		const FLinearColor& ShadowColor,
		const FLinearColor& RimColor,
		float RimPower = 3.0f);

	/**
	 * 为怪物 Actor 创建并应用 NPR 材质（使用预设调色板）
	 * @param Enemy    目标敌人 Actor（须有 SkeletalMeshComponent）
	 * @param Palette  预设配色索引 (0=CandyZombie, 1=Gingerbread, 2=ShadowNinja, 3=ArmoredGum)
	 *                 使用 uint8 而非枚举，避免 Python binding 枚举转换问题
	 */
	UFUNCTION(BlueprintCallable, Category = "NPR|Monster")
	static void ApplyNPRMaterialToMonster(AActor* Enemy, uint8 Palette);

	/**
	 * 获取或创建 NPR 父材质（含完整 Expression 图）
	 * 第一次调用时构建并缓存，后续直接返回
	 * @return NPR 父材质指针，失败返回 nullptr
	 */
	static UMaterial* GetOrCreateNPRParentMaterial();

	// ─── Outline (Inverted Hull) 材质 ────────────────────────────────

	/**
	 * 创建描边材质实例
	 *
	 * 原理：
	 *   - TwoSided = true（渲染双面）
	 *   - BlendMode = Masked（通过 OpacityMask 裁剪正面）
	 *   - WPO = Normal * OutlineWidth（将顶点沿法线挤出）
	 *   - 正面 (NdotV >= 0)：OpacityMask = 0 → 被裁剪
	 *   - 背面 (NdotV <  0)：OpacityMask = 1 → 可见，形成轮廓
	 *
	 * @param WorldContextObject UWorld 上下文
	 * @param MaterialName       MID 名称
	 * @param OutlineColor       描边颜色
	 * @param OutlineWidth       描边宽度 (cm, 建议 1.0~5.0)
	 * @return 创建的 MID，失败返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "NPR|Outline",
		meta = (WorldContext = "WorldContextObject"))
	static UMaterialInstanceDynamic* CreateOutlineMaterialInstance(
		UObject* WorldContextObject,
		const FString& MaterialName,
		const FLinearColor& OutlineColor,
		float OutlineWidth = 2.0f);

	/**
	 * 获取或创建描边父材质
	 */
	static UMaterial* GetOrCreateOutlineParentMaterial();

	// ─── 应用描边到怪物 ──────────────────────────────────────────────

	/**
	 * 为怪物生成描边子 Actor（SpawnAttachment）
	 *
	 * 描边需要单独的 SkeletalMeshComponent（相同的 Mesh + AnimBP）
	 * 使用 Outline 材质渲染，作为子 Actor Attach 到怪物身上
	 *
	 * @param Monster        目标怪物 Actor
	 * @param OutlineColor   描边颜色
	 * @param OutlineWidth   描边宽度
	 * @return 创建的描边 Actor，失败返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "NPR|Outline")
	static AActor* SpawnOutlineAttachment(
		AActor* Monster,
		const FLinearColor& OutlineColor = FLinearColor::Black,
		float OutlineWidth = 2.0f);

	// ─── 调色板查询 ──────────────────────────────────────────────────

	/**
	 * 获取预设配色
	 * @param Palette       调色板枚举
	 * @param OutBase       [Out] 受光色
	 * @param OutShadow     [Out] 阴影色
	 * @param OutRim        [Out] Rim 色
	 */
	static void GetMonsterPalette(
		ENPRMonsterPalette Palette,
		FLinearColor& OutBase,
		FLinearColor& OutShadow,
		FLinearColor& OutRim);

private:
	// ─── 内部：材质图构建（WITH_EDITOR 或 EDITORONLY_DATA）────────────

#if WITH_EDITORONLY_DATA
	/**
	 * 构建 NPR 父材质的 Expression 节点图
	 * @param Mat 目标材质（须已 NewObject，ShadingModel=MSM_Unlit）
	 */
	static void BuildNPRMaterialGraph(UMaterial* Mat);

	/**
	 * 构建描边父材质的 Expression 节点图
	 * @param Mat 目标材质
	 */
	static void BuildOutlineMaterialGraph(UMaterial* Mat);

	/**
	 * 辅助：创建表达式并加入材质的 ExpressionCollection
	 */
	template <typename ExprClass>
	static ExprClass* AddExpression(UMaterial* Mat, const TCHAR* DebugName = nullptr);
#endif

	/** 缓存的 NPR 父材质 */
	static UMaterial* CachedNPRParentMaterial;

	/** 缓存的描边父材质 */
	static UMaterial* CachedOutlineParentMaterial;
};
