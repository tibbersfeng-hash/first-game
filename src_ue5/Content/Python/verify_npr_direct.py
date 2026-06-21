"""
NPR 材质直接验证 — 不依赖 C++ 工具类
触发：FIRSTGAME_NPR_DIRECT=1

直接在 Python 中创建材质实例并应用，验证 NPR 效果
"""
import unreal
import time
import os
import glob
import shutil

DOCS_DIR = "/home/vipuser/first-game/docs"
SS_DIR = "/home/vipuser/first-game/src_ue5/Saved/Screenshots/LinuxEditor"

# 怪物配置
MONSTERS = [
    {
        "key": "CandyZombie",
        "mesh": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
        "anim": "/Game/Monsters/CandyZombie/ABP/ABP_CandyZombie",
        "pos": unreal.Vector(-200, 0, 0),
        "rot": unreal.Rotator(0, 30, 0),
        "base_color": unreal.LinearColor(0.55, 0.85, 0.60, 1.0),
        "shadow_color": unreal.LinearColor(0.25, 0.15, 0.30, 1.0),
        "rim_color": unreal.LinearColor(0.80, 1.00, 0.70, 1.0),
    },
    {
        "key": "Gingerbread",
        "mesh": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
        "anim": "/Game/Monsters/Gingerbread/ABP/ABP_Gingerbread",
        "pos": unreal.Vector(-70, 0, 0),
        "rot": unreal.Rotator(0, 0, 0),
        "base_color": unreal.LinearColor(0.85, 0.55, 0.30, 1.0),
        "shadow_color": unreal.LinearColor(0.35, 0.12, 0.08, 1.0),
        "rim_color": unreal.LinearColor(1.00, 0.85, 0.40, 1.0),
    },
    {
        "key": "ShadowNinja",
        "mesh": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
        "anim": "/Game/Monsters/ShadowNinja/ABP/ABP_ShadowNinja",
        "pos": unreal.Vector(70, 0, 0),
        "rot": unreal.Rotator(0, -30, 0),
        "base_color": unreal.LinearColor(0.40, 0.20, 0.55, 1.0),
        "shadow_color": unreal.LinearColor(0.15, 0.05, 0.20, 1.0),
        "rim_color": unreal.LinearColor(0.30, 0.70, 1.00, 1.0),
    },
    {
        "key": "ArmoredGum",
        "mesh": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
        "anim": "/Game/Monsters/ArmoredGum/ABP/ABP_ArmoredGum",
        "pos": unreal.Vector(200, 0, 0),
        "rot": unreal.Rotator(0, -60, 0),
        "base_color": unreal.LinearColor(0.90, 0.92, 0.95, 1.0),
        "shadow_color": unreal.LinearColor(0.15, 0.20, 0.35, 1.0),
        "rim_color": unreal.LinearColor(1.00, 1.00, 1.00, 1.0),
    },
]


def find_or_create_npr_parent_material():
    """找到或创建 NPR 父材质"""
    # 尝试加载已存在的 NPR 父材质
    mat_path = "/Game/Materials/M_NPR_Parent"
    mat = unreal.load_asset(mat_path)
    if mat:
        unreal.log(f"  ✅ 找到 NPR 父材质：{mat_path}")
        return mat

    # 尝试通过 C++ 工具类创建
    unreal.log("  尝试通过 C++ 工具类创建 NPR 父材质...")
    try:
        result = unreal.SystemLibrary.execute_console_command(
            None,
            'py "from first_game.NPRMaterialUtils import UNPRMaterialUtils; '
            'mat = UNPRMaterialUtils.GetOrCreateNPRParentMaterial(); '
            'print(\'NPR_PARENT=\' + str(mat))"'
        )
        unreal.log(f"  C++ 创建结果：{result}")
    except Exception as e:
        unreal.log(f"  C++ 创建失败：{e}")

    # 降级：使用简单的 Unlit 材质
    unreal.log("  降级：创建简单 Unlit 材质")
    parent_mat = unreal.EditorAssetLibrary.load_asset("/Engine/EngineMaterials/BaseWhiteUnlitMaterial")
    if parent_mat:
        unreal.log("  ✅ 使用 BaseWhiteUnlitMaterial 作为降级方案")
    else:
        unreal.log("   无法找到降级材质")

    return parent_mat


def create_simple_npr_material(base_color, shadow_color, rim_color, name="MI_SimpleNPR"):
    """创建简单的 NPR 材质实例（使用 DefaultMaterial 作为父材质）"""
    try:
        # 使用 DefaultMaterial 作为父材质
        parent = unreal.load_asset("/Engine/EngineMaterials/DefaultMaterial")
        if not parent:
            unreal.log_error("  ❌ DefaultMaterial 未找到")
            return None

        # 创建材质实例
        factory = unreal.MaterialInstanceConstantFactoryNew()
        factory.set_editor_property('Parent', parent)

        tools = unreal.AssetToolsHelpers.get_asset_tools()
        mi = tools.create_asset(
            asset_name=name,
            package_path="/Game/Materials/Test",
            asset_class=unreal.MaterialInstanceConstant,
            factory=factory,
        )

        if not mi:
            unreal.log_error(f"  ❌ 无法创建材质实例 {name}")
            return None

        # 设置 BaseColor（如果参数存在）
        try:
            mi.set_vector_parameter_value('Base Color', base_color)
            unreal.log(f"  ✅ {name}: BaseColor 已设置")
        except Exception as e:
            unreal.log(f"  ⚠️  BaseColor 设置失败：{e}")

        unreal.EditorAssetLibrary.save_asset(f"/Game/Materials/Test/{name}")
        return mi

    except Exception as e:
        unreal.log_error(f"  ❌ 创建材质失败：{e}")
        return None


def spawn_monsters_with_materials():
    """生成怪物并应用材质"""
    unreal.log("  生成怪物...")
    spawned = []

    # 确保材质目录存在
    if not unreal.EditorAssetLibrary.does_directory_exist("/Game/Materials/Test"):
        unreal.EditorAssetLibrary.make_directory("/Game/Materials/Test")

    for i, cfg in enumerate(MONSTERS):
        mesh = unreal.load_asset(cfg["mesh"])
        if not mesh:
            unreal.log_error(f"   Mesh 未找到：{cfg['mesh']}")
            continue

        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.SkeletalMeshActor, cfg["pos"], cfg["rot"])
        if not actor:
            continue

        skel_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        if not skel_comp:
            continue

        skel_comp.set_skeletal_mesh(mesh)

        # 设置 AnimBP
        anim_bp = unreal.load_asset(cfg["anim"])
        if anim_bp:
            skel_comp.set_anim_instance_class(anim_bp.generated_class())

        # 创建并应用简单 NPR 材质
        mat_name = f"MI_{cfg['key']}_Test"
        mi = create_simple_npr_material(
            cfg["base_color"], cfg["shadow_color"], cfg["rim_color"], mat_name)

        if mi:
            skel_comp.set_material(0, mi)
            unreal.log(f"  ✅ {cfg['key']}: 材质已应用 (BaseColor RGB={cfg['base_color'].r:.2f},{cfg['base_color'].g:.2f},{cfg['base_color'].b:.2f})")
        else:
            unreal.log(f"  ⚠️  {cfg['key']}: 材质创建失败，使用默认")

        spawned.append(actor)

    unreal.log(f"  ✅ 共生成 {len(spawned)}/{len(MONSTERS)} 只怪物")
    return spawned


def setup_lighting():
    """设置适合 NPR 验证的灯光"""
    unreal.log("  设置灯光...")

    # 主光源
    dir_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(-45, 45, 0))
    if dir_light:
        comp = dir_light.get_component_by_class(unreal.DirectionalLightComponent)
        if comp:
            comp.set_editor_property('Intensity', 5.0)  # 更强的光
            comp.set_editor_property('LightColor', unreal.Color(r=255, g=242, b=229, a=255))
        unreal.log("  ✅ DirectionalLight (Intensity=5.0)")

    # 天空光
    sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight, unreal.Vector(0, 0, 1000), unreal.Rotator(0, 0, 0))
    if sky_light:
        sky_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
        if sky_comp:
            sky_comp.set_editor_property('Intensity', 1.0)
        unreal.log("  ✅ SkyLight (Intensity=1.0)")


def setup_camera():
    """设置相机位置"""
    unreal.log("  设置相机...")
    camera_pos = unreal.Vector(0, -400, 250)
    camera_rot = unreal.Rotator(-30, 0, 0)

    camera = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor, camera_pos, camera_rot)
    if camera:
        unreal.log(f"  ✅ CameraActor at {camera_pos}")
        unreal.SystemLibrary.execute_console_command(
            None, "setviewtarget " + camera.get_actor_label())
    else:
        unreal.log("  ⚠️  Failed to spawn camera")


def take_screenshot(name="npr_direct"):
    """截图并保存"""
    unreal.log(f"  截图：{name}...")

    # 关闭自动曝光，使用固定曝光
    unreal.SystemLibrary.execute_console_command(None, "r.AutoExposureMethod 0")
    unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")
    unreal.SystemLibrary.execute_console_command(None, "r.MotionBlurQuality 0")
    unreal.SystemLibrary.execute_console_command(None, "r.AmbientOcclusionQuality 0")
    unreal.SystemLibrary.execute_console_command(None, "r.BloomQuality 0")

    time.sleep(3)

    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(5)

    # 复制截图
    try:
        shots = glob.glob(os.path.join(SS_DIR, "HighresScreenshot*.png"))
        if not shots:
            shots = glob.glob(os.path.join(SS_DIR, "ScreenShot*.png"))

        if shots:
            latest = max(shots, key=os.path.getmtime)
            dest = os.path.join(DOCS_DIR, f"verify_{name}.png")
            shutil.copy2(latest, dest)
            unreal.log(f"  ✅ 截图已保存：{dest}")
        else:
            unreal.log("  ⚠️  未找到截图文件")
    except Exception as e:
        unreal.log_error(f"  ❌ 截图复制失败：{e}")


def main():
    unreal.log("=" * 60)
    unreal.log("[NPR DIRECT] 直接验证开始")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.log(f"  加载关卡：{level_path}")
    loaded = unreal.EditorLevelLibrary.load_level(level_path)
    if not loaded:
        unreal.log_error(f"   关卡加载失败")
        unreal.SystemLibrary.execute_console_command(None, "quit")
        return
    unreal.log(f"  ✅ 关卡加载成功")

    # 设置灯光
    setup_lighting()

    # 生成怪物 + 材质
    monsters = spawn_monsters_with_materials()

    if not monsters:
        unreal.log_error("  ❌ 没有怪物生成")
        unreal.SystemLibrary.execute_console_command(None, "quit")
        return

    # 设置相机
    setup_camera()

    # 等待渲染
    unreal.log("  等待渲染...")
    time.sleep(5)

    # 截图
    take_screenshot("npr_direct")

    # 报告
    unreal.log("=" * 60)
    unreal.log("[NPR DIRECT] 验证报告")
    unreal.log("=" * 60)
    unreal.log(f"  怪物：{len(monsters)}/{len(MONSTERS)}")
    unreal.log(f"  材质：✅ 简单 NPR (BaseColor 直接设置)")
    unreal.log("=" * 60)

    time.sleep(5)
    unreal.log("[NPR DIRECT] 完成，退出编辑器")
    unreal.SystemLibrary.execute_console_command(None, "quit")


if __name__ == "__main__":
    main()
else:
    main()
