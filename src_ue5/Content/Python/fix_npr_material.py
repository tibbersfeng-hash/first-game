"""
修复 NPR 材质 - 重新创建并正确连接 EmissiveColor
"""
import unreal

MATERIALS_DIR = "/Game/Materials"
NPR_PARENT_PATH = f"{MATERIALS_DIR}/M_NPR_Parent"

def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def create_simple_test_material():
    """创建简单测试材质：常量颜色 → EmissiveColor"""
    unreal.log("[FIX] 创建简单测试材质...")
    ensure_dir(MATERIALS_DIR)

    # 删除旧资产
    if unreal.EditorAssetLibrary.does_asset_exist(NPR_PARENT_PATH):
        unreal.EditorAssetLibrary.delete_asset(NPR_PARENT_PATH)

    # 创建材质
    factory = unreal.MaterialFactoryNew()
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="M_NPR_Parent",
        package_path=MATERIALS_DIR,
        asset_class=unreal.Material,
        factory=factory
    )
    if not mat:
        unreal.log_error("[FIX]  创建失败")
        return None

    # 设置 Unlit
    mat.set_editor_property('ShadingModel', unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property('bUsedWithSkeletalMesh', True)

    # 创建常量颜色节点 (青色 = 0, 1, 1)
    const_color = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, 0, 0)
    const_color.set_editor_property('Constant', unreal.LinearColor(0.0, 1.0, 1.0, 1.0))  # 青色
    unreal.log("[FIX]  创建常量颜色节点 (青色)")

    # 连接到 EmissiveColor
    result = unreal.MaterialEditingLibrary.connect_material_property(
        const_color, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.log(f"[FIX]  连接 EmissiveColor (result={result})")

    # 验证连接
    input_node = unreal.MaterialEditingLibrary.get_material_property_input_node(
        mat, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if input_node:
        unreal.log(f"[FIX]  ✅ EmissiveColor 已连接：{input_node.get_class().get_name()}")
    else:
        unreal.log_error("[FIX]  ❌ EmissiveColor 未连接！")

    # 保存
    unreal.EditorAssetLibrary.save_asset(NPR_PARENT_PATH)
    unreal.log(f"[FIX]  ✅ 保存成功：{NPR_PARENT_PATH}")

    return mat

def main():
    unreal.log("=" * 60)
    unreal.log("[FIX] 修复 NPR 材质连接")
    unreal.log("=" * 60)

    mat = create_simple_test_material()
    if mat:
        unreal.log("[FIX] === 完成 ===")
    else:
        unreal.log_error("[FIX] === 失败 ===")

    unreal.SystemLibrary.execute_console_command(None, "quit")

if __name__ == "__main__":
    main()
else:
    main()
