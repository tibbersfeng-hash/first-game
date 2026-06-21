"""
完整 NPR 材质解决方案 - 包含 Shader 强制编译
触发：FIRSTGAME_FINAL_NPR=1
"""
import unreal
import time

MATERIALS_DIR = "/Game/Materials"
NPR_PARENT_PATH = f"{MATERIALS_DIR}/M_NPR_Parent"

def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def create_npr_material_with_shader_compile():
    """创建 NPR 材质并强制编译 Shader"""
    unreal.log("[FINAL] === 创建 NPR 材质（含 Shader 编译）===")
    ensure_dir(MATERIALS_DIR)

    # 删除旧资产
    if unreal.EditorAssetLibrary.does_asset_exist(NPR_PARENT_PATH):
        unreal.EditorAssetLibrary.delete_asset(NPR_PARENT_PATH)
        unreal.log("[FINAL] 已删除旧资产")

    # 1) 创建材质
    factory = unreal.MaterialFactoryNew()
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="M_NPR_Parent",
        package_path=MATERIALS_DIR,
        asset_class=unreal.Material,
        factory=factory
    )
    if not mat:
        unreal.log_error("[FINAL] ❌ 创建失败")
        return None

    # 2) 设置属性
    mat.set_editor_property('ShadingModel', unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property('bUsedWithSkeletalMesh', True)
    mat.set_editor_property('bUsedWithStaticLighting', False)
    unreal.log("[FINAL] ✅ 材质属性设置完成 (MSM_Unlit)")

    # 3) 创建参数节点
    unreal.log("[FINAL] 创建参数节点...")

    base_color = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionVectorParameter, -400, -200)
    base_color.set_editor_property('ParameterName', 'BaseColor')
    base_color.set_editor_property('DefaultValue', unreal.LinearColor(0.8, 0.6, 0.4, 1.0))
    unreal.log("  ✅ BaseColor")

    shadow_color = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionVectorParameter, -400, 0)
    shadow_color.set_editor_property('ParameterName', 'ShadowColor')
    shadow_color.set_editor_property('DefaultValue', unreal.LinearColor(0.3, 0.15, 0.2, 1.0))
    unreal.log("  ✅ ShadowColor")

    rim_color = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionVectorParameter, -400, 200)
    rim_color.set_editor_property('ParameterName', 'RimColor')
    rim_color.set_editor_property('DefaultValue', unreal.LinearColor(1.0, 0.9, 0.7, 1.0))
    unreal.log("  ✅ RimColor")

    shadow_threshold = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionScalarParameter, -400, 400)
    shadow_threshold.set_editor_property('ParameterName', 'ShadowThreshold')
    shadow_threshold.set_editor_property('DefaultValue', 0.3)
    unreal.log("  ✅ ShadowThreshold")

    rim_power = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionScalarParameter, -400, 600)
    rim_power.set_editor_property('ParameterName', 'RimPower')
    rim_power.set_editor_property('DefaultValue', 3.0)
    unreal.log("  ✅ RimPower")

    # 光方向常量
    import math
    lx, ly, lz = 0.5, 0.3, 0.8
    length = math.sqrt(lx*lx + ly*ly + lz*lz)
    light_dir = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -400, 800)
    light_dir.set_editor_property('Constant', unreal.LinearColor(lx/length, ly/length, lz/length, 1.0))
    unreal.log("  ✅ LightDirection")

    # 4) 核心表达式
    unreal.log("[FINAL] 构建表达式图...")

    vert_normal = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionVertexNormalWS, -200, -200)
    cam_vec = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionCameraVectorWS, -200, 400)

    # NdotL
    ndotl = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionDotProduct, 0, -200)
    unreal.MaterialEditingLibrary.connect_material_expressions(vert_normal, '', ndotl, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(light_dir, '', ndotl, 'B')

    # ShadowMask
    shadow_if = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionIf, 200, -200)
    unreal.MaterialEditingLibrary.connect_material_expressions(ndotl, '', shadow_if, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(shadow_threshold, '', shadow_if, 'B')

    one_const = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant, 0, -400)
    one_const.set_editor_property('R', 1.0)
    zero_const = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant, 0, 0)
    zero_const.set_editor_property('R', 0.0)
    unreal.MaterialEditingLibrary.connect_material_expressions(one_const, '', shadow_if, 'AGreaterThanB')
    unreal.MaterialEditingLibrary.connect_material_expressions(one_const, '', shadow_if, 'AEqualsB')
    unreal.MaterialEditingLibrary.connect_material_expressions(zero_const, '', shadow_if, 'ALessThanB')

    # SurfaceColor
    color_lerp = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionLinearInterpolate, 400, -100)
    unreal.MaterialEditingLibrary.connect_material_expressions(shadow_color, '', color_lerp, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(base_color, '', color_lerp, 'B')
    unreal.MaterialEditingLibrary.connect_material_expressions(shadow_if, '', color_lerp, 'Alpha')

    # NdotV → Rim
    ndotv = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionDotProduct, 200, 400)
    unreal.MaterialEditingLibrary.connect_material_expressions(vert_normal, '', ndotv, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(cam_vec, '', ndotv, 'B')

    one_minus = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionOneMinus, 400, 400)
    unreal.MaterialEditingLibrary.connect_material_expressions(ndotv, '', one_minus, 'Input')

    rim_pow = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionPower, 600, 400)
    unreal.MaterialEditingLibrary.connect_material_expressions(one_minus, '', rim_pow, 'Base')
    unreal.MaterialEditingLibrary.connect_material_expressions(rim_power, '', rim_pow, 'Exponent')

    rim_contrib = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionMultiply, 800, 300)
    unreal.MaterialEditingLibrary.connect_material_expressions(rim_pow, '', rim_contrib, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(rim_color, '', rim_contrib, 'B')

    # FinalColor
    final_add = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionAdd, 1000, 100)
    unreal.MaterialEditingLibrary.connect_material_expressions(color_lerp, '', final_add, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(rim_contrib, '', final_add, 'B')

    unreal.log("  ✅ 表达式图构建完成")

    # 5) 连接 EmissiveColor
    unreal.log("[FINAL] 连接 EmissiveColor...")
    result = unreal.MaterialEditingLibrary.connect_material_property(
        final_add, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.log(f"  ✅ 连接结果：{result}")

    # 验证
    input_node = unreal.MaterialEditingLibrary.get_material_property_input_node(
        mat, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if input_node:
        unreal.log(f"  ✅ EmissiveColor 输入：{input_node.get_class().get_name()}")
    else:
        unreal.log_error("  ❌ EmissiveColor 未连接！")

    # 6) 保存
    unreal.log("[FINAL] 保存材质...")
    unreal.EditorAssetLibrary.save_asset(NPR_PARENT_PATH)
    unreal.log(f"[FINAL] ✅ 保存成功：{NPR_PARENT_PATH}")

    # 7) 强制 Shader 编译（关键步骤）
    unreal.log("[FINAL] 等待 Shader 编译...")
    time.sleep(5)  # 给编辑器时间处理保存

    # 使用控制台命令强制编译
    unreal.SystemLibrary.execute_console_command(None, "recompile shaders changed")
    unreal.log("[FINAL] ✅ 已提交 Shader 编译命令")

    # 等待编译完成
    unreal.log("[FINAL] 等待 30 秒让 Shader 编译完成...")
    time.sleep(30)

    # 验证表达式数量
    num_expr = unreal.MaterialEditingLibrary.get_num_material_expressions(mat)
    unreal.log(f"[FINAL] 表达式节点数：{num_expr}")

    return mat

def main():
    unreal.log("=" * 60)
    unreal.log("[FINAL] NPR 材质完整解决方案")
    unreal.log("=" * 60)

    mat = create_npr_material_with_shader_compile()

    if mat:
        unreal.log("[FINAL] === 完成 ===")
        unreal.log("[FINAL] 现在可以运行测试脚本验证颜色")
    else:
        unreal.log_error("[FINAL] === 失败 ===")

    unreal.SystemLibrary.execute_console_command(None, "quit")

if __name__ == "__main__":
    main()
else:
    main()
