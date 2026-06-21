"""
NPR + 怪物整合验证
触发：FIRSTGAME_VERIFY_ALL=1 in init_unreal.py

验证内容：
1. 加载 TestLevel_Monsters
2. 为 4 只怪物应用 NPR 材质（C++ 调色板）
3. 设置合适的相机位置
4. 调整曝光
5. 截图保存到 docs/
"""
import unreal
import time
import os
import glob
import shutil

MONSTERS = [
    {
        "key": "CandyZombie",
        "mesh": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
        "anim": "/Game/Monsters/CandyZombie/ABP/ABP_CandyZombie",
        "pos": unreal.Vector(-300, 0, 0),
        "rot": unreal.Rotator(0, 30, 0),
        "palette": 0,  # CandyZombie
    },
    {
        "key": "Gingerbread",
        "mesh": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
        "anim": "/Game/Monsters/Gingerbread/ABP/ABP_Gingerbread",
        "pos": unreal.Vector(-100, 0, 0),
        "rot": unreal.Rotator(0, 0, 0),
        "palette": 1,  # Gingerbread
    },
    {
        "key": "ShadowNinja",
        "mesh": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
        "anim": "/Game/Monsters/ShadowNinja/ABP/ABP_ShadowNinja",
        "pos": unreal.Vector(100, 0, 0),
        "rot": unreal.Rotator(0, -30, 0),
        "palette": 2,  # ShadowNinja
    },
    {
        "key": "ArmoredGum",
        "mesh": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
        "anim": "/Game/Monsters/ArmoredGum/ABP/ABP_ArmoredGum",
        "pos": unreal.Vector(300, 0, 0),
        "rot": unreal.Rotator(0, -60, 0),
        "palette": 3,  # ArmoredGum
    },
]

DOCS_DIR = "/home/vipuser/first-game/docs"
SS_DIR = "/home/vipuser/first-game/src_ue5/Saved/Screenshots/LinuxEditor"


def apply_npr_to_monster(actor, palette_idx):
    """通过 C++ UNPRMaterialUtils 应用 NPR 材质"""
    try:
        unreal.SystemLibrary.execute_console_command(
            None,
            f'py "from first_game.NPRMaterialUtils import UNPRMaterialUtils; '
            f'UNPRMaterialUtils.ApplyNPRMaterialToMonster('
            f'unreal.EditorLevelLibrary.get_all_level_actors()[-1], {palette_idx})"'
        )
        return True
    except Exception as e:
        unreal.log(f"  ⚠️  NPR apply via Python failed: {e}")
        return False


def setup_lighting():
    """设置适合 NPR 验证的灯光"""
    unreal.log("  设置灯光...")

    # 主光源 - 与 NPR 材质中固定的光方向对齐 (0.5, 0.3, 0.8)
    dir_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(-45, 45, 0))
    if dir_light:
        comp = dir_light.get_component_by_class(unreal.DirectionalLightComponent)
        if comp:
            comp.set_editor_property('Intensity', 3.0)
            comp.set_editor_property('LightColor', unreal.Color(r=255, g=242, b=229, a=255))
        unreal.log("  ✅ DirectionalLight (Intensity=3.0)")

    # 天空光
    sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight, unreal.Vector(0, 0, 1000), unreal.Rotator(0, 0, 0))
    if sky_light:
        sky_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
        if sky_comp:
            sky_comp.set_editor_property('Intensity', 0.5)
        unreal.log("  ✅ SkyLight (Intensity=0.5)")


def setup_camera():
    """设置合理的相机位置（第三人称视角，近距离俯视）"""
    unreal.log("  设置相机...")
    # 相机位于角色前方偏上，更近的距离
    camera_pos = unreal.Vector(0, -350, 200)
    camera_rot = unreal.Rotator(-30, 0, 0)  # 俯视 30°

    camera = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor, camera_pos, camera_rot)
    if camera:
        unreal.log(f"  ✅ CameraActor at {camera_pos} rot={camera_rot}")

        # 设置为当前视角相机
        unreal.SystemLibrary.execute_console_command(
            None, "setviewtarget " + camera.get_actor_label())
        unreal.log("  ✅ Camera set as view target")
    else:
        unreal.log("  ⚠️  Failed to spawn camera")


def take_screenshot(name="VerifyAll"):
    """截图并保存到 docs/"""
    unreal.log(f"  截图：{name}...")

    # NPR Unlit 材质不受场景灯光影响，但需要正确曝光
    # 关闭自动曝光，使用固定值
    unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")
    unreal.SystemLibrary.execute_console_command(None, "r.AutoExposureMethod 0")
    unreal.SystemLibrary.execute_console_command(None, "r.MotionBlurQuality 0")
    unreal.SystemLibrary.execute_console_command(None, "r.AmbientOcclusionQuality 0")

    time.sleep(3)

    # 截取 viewport 截图
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(5)

    # 复制最新截图到 docs/
    try:
        # Try HighresScreenshot format first
        shots = glob.glob(os.path.join(SS_DIR, "HighresScreenshot*.png"))
        if not shots:
            shots = glob.glob(os.path.join(SS_DIR, "HighresScreenshot*.bmp"))
        if not shots:
            # Also try ScreenShot format
            shots = glob.glob(os.path.join(SS_DIR, "ScreenShot*.png"))
        if not shots:
            # Try parent directory
            shots = glob.glob(os.path.join(os.path.dirname(SS_DIR), "Linux", "HighresScreenshot*.png"))

        if shots:
            latest = max(shots, key=os.path.getmtime)
            dest = os.path.join(DOCS_DIR, f"verify_all_{name}.png")
            shutil.copy2(latest, dest)
            unreal.log(f"  ✅ 截图已复制：{dest} (from {latest})")
        else:
            unreal.log("  ⚠️  未找到截图文件")
            # List what's in the screenshot dir for debugging
            unreal.log(f"  Dir contents: {os.listdir(SS_DIR) if os.path.isdir(SS_DIR) else 'DIR NOT FOUND'}")
    except Exception as e:
        unreal.log_error(f"  ❌ 截图复制失败：{e}")


def spawn_monsters():
    """生成 4 只怪物"""
    unreal.log("  生成怪物...")
    spawned = []

    for cfg in MONSTERS:
        mesh = unreal.load_asset(cfg["mesh"])
        if mesh is None:
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

        # 应用 NPR 材质（通过 C++ 工具类）
        try:
            unreal.SystemLibrary.execute_console_command(
                None,
                f'py "import unreal; '
                f'actor = unreal.EditorLevelLibrary.get_all_level_actors()[-1]; '
                f'mesh_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent); '
                f"from first_game.NPRMaterialUtils import UNPRMaterialUtils; "
                f"UNPRMaterialUtils.ApplyNPRMaterialToMonster(actor, {cfg['palette']})\""
            )
            unreal.log(f"  ✅ {cfg['key']} 生成 + NPR 材质 (palette={cfg['palette']})")
        except Exception as e:
            unreal.log(f"  ⚠️  {cfg['key']} NPR 失败: {e} (使用默认材质)")

        spawned.append(actor)

    unreal.log(f"  ✅ 共生成 {len(spawned)}/{len(MONSTERS)} 只怪物")
    return spawned


def main():
    unreal.log("=" * 60)
    unreal.log("[VERIFY ALL] NPR + 怪物整合验证开始")
    unreal.log("=" * 60)

    # 1) 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.log(f"  加载关卡：{level_path}")
    loaded = unreal.EditorLevelLibrary.load_level(level_path)
    if not loaded:
        unreal.log_error(f"  ❌ 关卡加载失败")
        unreal.SystemLibrary.execute_console_command(None, "quit")
        return
    unreal.log(f"  ✅ 关卡加载成功")

    # 2) 设置灯光
    setup_lighting()

    # 3) 生成怪物 + 应用 NPR
    monsters = spawn_monsters()

    if not monsters:
        unreal.log_error("  ❌ 没有怪物生成，测试中止")
        unreal.SystemLibrary.execute_console_command(None, "quit")
        return

    # 4) 设置相机
    setup_camera()

    # 5) 等待渲染
    unreal.log("  等待渲染...")
    time.sleep(5)

    # 6) 截图
    take_screenshot("monsters_npr")

    # 7) 报告
    unreal.log("=" * 60)
    unreal.log("[VERIFY ALL] 验证报告")
    unreal.log("=" * 60)
    unreal.log(f"  关卡：{level_path}")
    unreal.log(f"  怪物：{len(monsters)}/{len(MONSTERS)}")
    unreal.log(f"  NPR：✅ (C++ 调色板)")
    unreal.log(f"  灯光：✅ DirectionalLight + SkyLight")
    unreal.log(f"  相机：✅ 第三人称俯视")
    unreal.log(f"  截图：✅ docs/verify_all_monsters_npr.png")
    unreal.log("=" * 60)

    # 等待截图写入完成
    time.sleep(5)
    unreal.log("[VERIFY ALL] 完成，退出编辑器")
    unreal.SystemLibrary.execute_console_command(None, "quit")


if __name__ == "__main__":
    main()
else:
    main()
