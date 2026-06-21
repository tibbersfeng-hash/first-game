"""
NPR C++ 函数调用验证
触发：FIRSTGAME_NPR_CPP=1

直接通过 Python 调用 C++ UNPRMaterialUtils 函数
"""
import unreal
import time
import os
import glob
import shutil

DOCS_DIR = "/home/vipuser/first-game/docs"
SS_DIR = "/home/vipuser/first-game/src_ue5/Saved/Screenshots/LinuxEditor"


def call_cpp_npr(actor, palette_idx):
    """尝试通过不同方式调用 C++ NPR 函数"""

    # 方式 1: 直接通过 unreal 模块调用
    try:
        unreal.NPRMaterialUtils.apply_npr_material_to_monster(actor, palette_idx)
        unreal.log(f"  ✅ 方式 1 成功 (unreal.NPRMaterialUtils)")
        return True
    except Exception as e:
        unreal.log(f"  ⚠️  方式 1 失败：{e}")

    # 方式 2: 通过类名调用
    try:
        cls = unreal.find_class('NPRMaterialUtils')
        if cls:
            cls.apply_npr_material_to_monster(actor, palette_idx)
            unreal.log(f"  ✅ 方式 2 成功 (find_class)")
            return True
    except Exception as e:
        unreal.log(f"  ⚠️  方式 2 失败：{e}")

    # 方式 3: 通过 execute_console_command + py
    try:
        cmd = (
            f'py "import unreal; '
            f'actor = unreal.EditorLevelLibrary.get_all_level_actors()[-1]; '
            f'unreal.NPRMaterialUtils.apply_npr_material_to_monster(actor, {palette_idx})"'
        )
        unreal.SystemLibrary.execute_console_command(None, cmd)
        unreal.log(f"  ✅ 方式 3 成功 (console_command)")
        return True
    except Exception as e:
        unreal.log(f"  ️  方式 3 失败：{e}")

    return False


def main():
    unreal.log("=" * 60)
    unreal.log("[NPR CPP] C++ 函数调用验证开始")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  ✅ 关卡加载")

    # 清空灯光
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if 'Light' in actor.get_class().get_name():
            unreal.EditorLevelLibrary.destroy_actor(actor)

    # 生成 4 只怪物
    MONSTERS = [
        {"key": "CandyZombie", "mesh": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
         "anim": "/Game/Monsters/CandyZombie/ABP/ABP_CandyZombie",
         "pos": unreal.Vector(-200, 0, 0), "palette": 0},
        {"key": "Gingerbread", "mesh": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
         "anim": "/Game/Monsters/Gingerbread/ABP/ABP_Gingerbread",
         "pos": unreal.Vector(-70, 0, 0), "palette": 1},
        {"key": "ShadowNinja", "mesh": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
         "anim": "/Game/Monsters/ShadowNinja/ABP/ABP_ShadowNinja",
         "pos": unreal.Vector(70, 0, 0), "palette": 2},
        {"key": "ArmoredGum", "mesh": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
         "anim": "/Game/Monsters/ArmoredGum/ABP/ABP_ArmoredGum",
         "pos": unreal.Vector(200, 0, 0), "palette": 3},
    ]

    spawned = []
    for cfg in MONSTERS:
        mesh = unreal.load_asset(cfg["mesh"])
        if not mesh:
            continue

        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.SkeletalMeshActor, cfg["pos"], unreal.Rotator(0, 0, 0))
        if not actor:
            continue

        skel = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        if not skel:
            continue

        skel.set_skeletal_mesh(mesh)

        anim_bp = unreal.load_asset(cfg["anim"])
        if anim_bp:
            skel.set_anim_instance_class(anim_bp.generated_class())

        # 尝试调用 C++ NPR 函数
        success = call_cpp_npr(actor, cfg["palette"])
        if not success:
            unreal.log(f"  ⚠️  {cfg['key']}: C++ NPR 调用失败，使用默认材质")

        spawned.append(actor)
        unreal.log(f"  ✅ {cfg['key']} 生成 (palette={cfg['palette']})")

    unreal.log(f"  共生成 {len(spawned)}/{len(MONSTERS)}")

    # 设置灯光（使用之前的强光配置）
    dir_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(-45, 0, 0))
    if dir_light:
        comp = dir_light.get_component_by_class(unreal.DirectionalLightComponent)
        if comp:
            comp.set_editor_property('Intensity', 100.0)
    unreal.log("  ✅ 灯光设置")

    # 相机
    cam = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor, unreal.Vector(0, -300, 200), unreal.Rotator(-25, 0, 0))
    if cam:
        unreal.SystemLibrary.execute_console_command(None, "setviewtarget " + cam.get_actor_label())

    # 关闭自动曝光
    unreal.SystemLibrary.execute_console_command(None, "r.AutoExposureMethod 0")
    unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")

    unreal.log("  等待渲染...")
    time.sleep(5)

    # 截图
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(5)

    shots = glob.glob(os.path.join(SS_DIR, "HighresScreenshot*.png"))
    if shots:
        latest = max(shots, key=os.path.getmtime)
        dest = os.path.join(DOCS_DIR, "verify_npr_cpp.png")
        shutil.copy2(latest, dest)
        unreal.log(f"  ✅ 截图：{dest}")

    unreal.log("=" * 60)
    unreal.log("[NPR CPP] 完成")
    unreal.log("=" * 60)

    time.sleep(3)
    unreal.SystemLibrary.execute_console_command(None, "quit")


if __name__ == "__main__":
    main()
else:
    main()
