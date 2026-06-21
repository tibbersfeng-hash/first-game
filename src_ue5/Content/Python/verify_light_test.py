"""
光照验证 — 确认场景渲染是否正常
触发：FIRSTGAME_LIGHT_TEST=1
"""
import unreal
import time
import os
import glob
import shutil

DOCS_DIR = "/home/vipuser/first-game/docs"
SS_DIR = "/home/vipuser/first-game/src_ue5/Saved/Screenshots/LinuxEditor"


def main():
    unreal.log("=" * 60)
    unreal.log("[LIGHT TEST] 光照验证开始")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  ✅ 关卡加载")

    # 清空场景中的现有灯光
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in all_actors:
        cls = actor.get_class().get_name()
        if 'Light' in cls:
            unreal.EditorLevelLibrary.destroy_actor(actor)
    unreal.log("  ✅ 清空现有灯光")

    # 生成一只怪物
    mesh = unreal.load_asset("/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie")
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkeletalMeshActor, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
    if actor:
        skel = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        if skel:
            skel.set_skeletal_mesh(mesh)
            # 强制设置材质为完全自发光（白色）
            # 创建一个简单的自发光材质
            mat = unreal.EditorAssetLibrary.load_asset("/Engine/EngineMaterials/BaseWhiteUnlitMaterial")
            if mat:
                skel.set_material(0, mat)
                unreal.log("  ✅ 应用 BaseWhiteUnlitMaterial (纯白自发光)")

    unreal.log("  ✅ 怪物生成")

    # 创建超强灯光
    dir_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(-45, 0, 0))
    if dir_light:
        comp = dir_light.get_component_by_class(unreal.DirectionalLightComponent)
        if comp:
            comp.set_editor_property('Intensity', 100.0)  # 超强光
            comp.set_editor_property('CastShadows', True)
        unreal.log("  ✅ DirectionalLight (Intensity=100)")

    # 天空光
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight, unreal.Vector(0, 0, 2000), unreal.Rotator(0, 0, 0))
    if sky:
        sc = sky.get_component_by_class(unreal.SkyLightComponent)
        if sc:
            sc.set_editor_property('Intensity', 5.0)
        unreal.log("  ✅ SkyLight (Intensity=5.0)")

    # 点光源（近距离补光）
    point_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PointLight, unreal.Vector(0, -200, 300), unreal.Rotator(0, 0, 0))
    if point_light:
        pl = point_light.get_component_by_class(unreal.PointLightComponent)
        if pl:
            pl.set_editor_property('Intensity', 50000.0)
            pl.set_editor_property('SourceRadius', 50.0)
        unreal.log("  ✅ PointLight (Intensity=50000)")

    # 相机
    cam = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor, unreal.Vector(0, -300, 200), unreal.Rotator(-25, 0, 0))
    if cam:
        unreal.SystemLibrary.execute_console_command(None, "setviewtarget " + cam.get_actor_label())
    unreal.log("  ✅ 相机设置")

    # 关闭自动曝光
    unreal.SystemLibrary.execute_console_command(None, "r.AutoExposureMethod 0")
    unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")
    unreal.SystemLibrary.execute_console_command(None, "r.BloomQuality 0")

    unreal.log("  等待渲染...")
    time.sleep(5)

    # 截图
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(5)

    # 复制
    shots = glob.glob(os.path.join(SS_DIR, "HighresScreenshot*.png"))
    if shots:
        latest = max(shots, key=os.path.getmtime)
        dest = os.path.join(DOCS_DIR, "verify_light_test.png")
        shutil.copy2(latest, dest)
        unreal.log(f"  ✅ 截图：{dest}")

    unreal.log("=" * 60)
    unreal.log("[LIGHT TEST] 完成")
    unreal.log("=" * 60)

    time.sleep(3)
    unreal.SystemLibrary.execute_console_command(None, "quit")


if __name__ == "__main__":
    main()
else:
    main()
