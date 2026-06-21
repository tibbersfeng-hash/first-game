"""
在 UE5 Editor 中创建完整的 NPR 卡通材质资产。
触发方式: FIRSTGAME_BUILD_NPR_MATERIAL=1

使用 MaterialEditingLibrary 构建完整的 2-tone + Rim Light 表达式图，
保存为 /Game/Materials/M_NPR_Parent。
"""
import unreal

MATERIALS_DIR = "/Game/Materials"
NPR_PARENT_NAME = "M_NPR_Parent"
NPR_PARENT_PATH = f"{MATERIALS_DIR}/{NPR_PARENT_NAME}"


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def build_npr_material():
    unreal.log("[NPR_BUILD] === 开始构建 NPR 材质 ===")
    ensure_dir(MATERIALS_DIR)

    # 删除旧资产
    if unreal.EditorAssetLibrary.does_asset_exist(NPR_PARENT_PATH):
        unreal.log("[NPR_BUILD] 删除旧资产...")
        unreal.EditorAssetLibrary.delete_asset(NPR_PARENT_PATH)

    # 1) 创建材质
    factory = unreal.MaterialFactoryNew()
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=NPR_PARENT_NAME,
        package_path=MATERIALS_DIR,
        asset_class=unreal.Material,
        factory=factory
    )
    if not mat:
        unreal.log_error("[NPR_BUILD]  创建材质失败")
        return None
    unreal.log("[NPR_BUILD] ✅ 材质创建成功")

    # 2) 设置属性
    mat.set_editor_property('ShadingModel', unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property('bUsedWithSkeletalMesh', True)
    mat.set_editor_property('bUsedWithStaticLighting', False)
    unreal.log("[NPR_BUILD] 材质属性：MSM_Unlit + SkeletalMesh")

    # 3) 创建参数节点
    unreal.log("[NPR_BUILD] 创建参数节点...")

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

    import math
    lx, ly, lz = 0.5, 0.3, 0.8
    length = math.sqrt(lx*lx + ly*ly + lz*lz)
    light_dir = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -400, 800)
    light_dir.set_editor_property('Constant', unreal.LinearColor(lx/length, ly/length, lz/length, 1.0))
    unreal.log("  ✅ LightDirection")

    # 4) 核心表达式
    unreal.log("[NPR_BUILD] 创建核心表达式...")

    vert_normal = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionVertexNormalWS, -200, -200)
    cam_vec = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionCameraVectorWS, -200, 400)

    # NdotL
    ndotl = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionDotProduct, 0, -200)
    unreal.MaterialEditingLibrary.connect_material_expressions(vert_normal, '', ndotl, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(light_dir, '', ndotl, 'B')
    unreal.log("  ✅ NdotL")

    # ShadowMask = If
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
    unreal.log("  ✅ ShadowMask")

    # SurfaceColor = Lerp
    color_lerp = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionLinearInterpolate, 400, -100)
    unreal.MaterialEditingLibrary.connect_material_expressions(shadow_color, '', color_lerp, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(base_color, '', color_lerp, 'B')
    unreal.MaterialEditingLibrary.connect_material_expressions(shadow_if, '', color_lerp, 'Alpha')
    unreal.log("  ✅ SurfaceColor")

    # NdotV
    ndotv = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionDotProduct, 200, 400)
    unreal.MaterialEditingLibrary.connect_material_expressions(vert_normal, '', ndotv, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(cam_vec, '', ndotv, 'B')

    # 1 - NdotV
    one_minus = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionOneMinus, 400, 400)
    unreal.MaterialEditingLibrary.connect_material_expressions(ndotv, '', one_minus, 'Input')

    # RimFactor = Power
    rim_pow = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionPower, 600, 400)
    unreal.MaterialEditingLibrary.connect_material_expressions(one_minus, '', rim_pow, 'Base')
    unreal.MaterialEditingLibrary.connect_material_expressions(rim_power, '', rim_pow, 'Exponent')

    # RimContrib = Multiply
    rim_contrib = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionMultiply, 800, 300)
    unreal.MaterialEditingLibrary.connect_material_expressions(rim_pow, '', rim_contrib, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(rim_color, '', rim_contrib, 'B')
    unreal.log("  ✅ RimContrib")

    # FinalColor = Add
    final_add = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionAdd, 1000, 100)
    unreal.MaterialEditingLibrary.connect_material_expressions(color_lerp, '', final_add, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(rim_contrib, '', final_add, 'B')
    unreal.log("  ✅ FinalColor")

    # 5) 连接 EmissiveColor
    unreal.log("[NPR_BUILD] 连接 EmissiveColor...")
    result = unreal.MaterialEditingLibrary.connect_material_property(
        final_add, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.log(f"  ✅ EmissiveColor 连接 (result={result})")

    # 验证
    emissive_input = unreal.MaterialEditingLibrary.get_material_property_input_node(
        mat, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if emissive_input:
        unreal.log(f"  ✅ EmissiveColor 输入节点：{emissive_input.get_class().get_name()}")
    else:
        unreal.log_warning("  ⚠️ EmissiveColor 输入节点为空！")

    # 6) 保存
    unreal.log("[NPR_BUILD] 保存材质...")
    unreal.EditorAssetLibrary.save_asset(NPR_PARENT_PATH)
    unreal.log(f"[NPR_BUILD] ✅ 材质已保存：{NPR_PARENT_PATH}")

    # 7) 触发 Shader 编译
    unreal.log("[NPR_BUILD] 触发 Shader 编译...")
    try:
        mat.post_edit_change()
        unreal.log("  ✅ post_edit_change() 已调用")
    except Exception as e:
        unreal.log_warning(f"  ⚠️ post_edit_change() 失败：{e}")

    num_expr = unreal.MaterialEditingLibrary.get_num_material_expressions(mat)
    unreal.log(f"[NPR_BUILD] 表达式节点数：{num_expr}")
    return mat


def main():
    unreal.log("=" * 60)
    unreal.log("[NPR_BUILD] 构建 NPR 卡通材质资产")
    unreal.log("=" * 60)

    mat = build_npr_material()
    if mat:
        unreal.log("[NPR_BUILD] === 完成 ===")
    else:
        unreal.log_error("[NPR_BUILD] === 失败 ===")

    unreal.SystemLibrary.execute_console_command(None, "quit")


if __name__ == "__main__":
    main()
else:
    main()
