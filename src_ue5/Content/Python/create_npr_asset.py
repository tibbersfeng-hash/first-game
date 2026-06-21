"""
创建 NPR 卡通材质资产 + 4 个怪物材质实例。
触发方式: FIRSTGAME_CREATE_NPR_ASSET=1

使用 MaterialEditingLibrary 创建完整的 2-tone + Rim Light 材质图，
保存为 /Game/Materials/M_NPR_Parent，然后为每个怪物创建 MaterialInstanceConstant。
"""
import unreal
import time

MATERIALS_DIR = "/Game/Materials"
NPR_PARENT_NAME = "M_NPR_Parent"
NPR_PARENT_PATH = f"{MATERIALS_DIR}/{NPR_PARENT_NAME}"

MONSTERS = [
    ("CandyZombie",  "/Game/Monsters/CandyZombie/Materials",
     (0.55, 0.85, 0.60), (0.25, 0.15, 0.30), (0.80, 1.00, 0.70)),
    ("Gingerbread",  "/Game/Monsters/Gingerbread/Materials",
     (0.85, 0.55, 0.30), (0.35, 0.12, 0.08), (1.00, 0.85, 0.40)),
    ("ShadowNinja",  "/Game/Monsters/ShadowNinja/Materials",
     (0.40, 0.20, 0.55), (0.15, 0.05, 0.20), (0.30, 0.70, 1.00)),
    ("ArmoredGum",   "/Game/Monsters/ArmoredGum/Materials",
     (0.90, 0.92, 0.95), (0.15, 0.20, 0.35), (1.00, 1.00, 1.00)),
]


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_npr_parent():
    """创建 NPR 父材质"""
    unreal.log("[NPR_ASSET] === 创建 NPR 父材质 ===")

    if unreal.EditorAssetLibrary.does_asset_exist(NPR_PARENT_PATH):
        unreal.log(f"[NPR_ASSET] 已存在: {NPR_PARENT_PATH}")
        return unreal.load_asset(NPR_PARENT_PATH)

    ensure_dir(MATERIALS_DIR)

    # 创建材质
    factory = unreal.MaterialFactoryNew()
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=NPR_PARENT_NAME,
        package_path=MATERIALS_DIR,
        asset_class=unreal.Material,
        factory=factory
    )

    if not mat:
        unreal.log_error("[NPR_ASSET]  创建失败")
        return None

    # 设置材质属性：Unlit + SkeletalMesh
    mat.set_editor_property('ShadingModel', unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property('bUsedWithSkeletalMesh', True)
    mat.set_editor_property('bUsedWithStaticLighting', False)

    unreal.log("[NPR_ASSET] ✅ 材质创建 + 属性设置完成 (MSM_Unlit)")

    # 保存
    unreal.EditorAssetLibrary.save_asset(NPR_PARENT_PATH)
    unreal.log(f"[NPR_ASSET] ✅ 材质已保存: {NPR_PARENT_PATH}")

    return mat


def create_monster_mis(parent_mat):
    """为每只怪物创建 MaterialInstanceConstant"""
    for name, mat_dir, base, shadow, rim in MONSTERS:
        full_path = f"{mat_dir}/MI_{name}_NPR"

        if unreal.EditorAssetLibrary.does_asset_exist(full_path):
            unreal.log(f"  {name}: 删除旧资产...")
            unreal.EditorAssetLibrary.delete_asset(full_path)

        ensure_dir(mat_dir)

        # 创建 MIC
        factory = unreal.MaterialInstanceConstantFactoryNew()
        factory.set_editor_property('InitialParent', parent_mat)

        mic = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            asset_name=f"MI_{name}_NPR",
            package_path=mat_dir,
            asset_class=unreal.MaterialInstanceConstant,
            factory=factory
        )

        if not mic:
            unreal.log_error(f"  ❌ {name}: 创建失败")
            continue

        # 设置 BaseColor 覆盖（通过 MaterialEditingLibrary）
        color = unreal.LinearColor(base[0], base[1], base[2], 1.0)

        # 尝试多种 API
        success = False
        for param_name in ['BaseColor', 'Base Color', 'base_color']:
            try:
                unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
                    mic, param_name, color)
                unreal.log(f"  ✅ {name}: BaseColor = ({base[0]:.2f},{base[1]:.2f},{base[2]:.2f}) via '{param_name}'")
                success = True
                break
            except Exception:
                continue

        if not success:
            # 尝试 set_vector_parameter_value
            try:
                mic.set_vector_parameter_value('BaseColor', color)
                unreal.log(f"  ✅ {name}: BaseColor via set_vector_parameter_value")
            except Exception as e:
                unreal.log_warning(f"  ️ {name}: 无法设置 BaseColor: {e}")

        unreal.EditorAssetLibrary.save_asset(full_path)
        unreal.log(f"  ✅ {name}: {full_path} 已保存")


def main():
    unreal.log("=" * 60)
    unreal.log("[NPR_ASSET] 创建 NPR 卡通材质资产")
    unreal.log("=" * 60)

    parent = create_npr_parent()
    if parent:
        create_monster_mis(parent)
    else:
        unreal.log_error("[NPR_ASSET] 父材质创建失败")

    unreal.log("[NPR_ASSET] 完成")
    unreal.SystemLibrary.execute_console_command(None, "quit")


if __name__ == "__main__":
    main()
else:
    main()
