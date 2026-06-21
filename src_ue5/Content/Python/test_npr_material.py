"""
Apply NPR material to monsters and take a screenshot.
Triggered via FIRSTGAME_NPR_TEST=1 in init_unreal.py.

流程：
1. 加载 TestLevel_Monsters
2. 生成 4 只糖果地牢怪物（SkeletalMeshActor）
3. 添加 DirectionalLight + SkyLight
4. 通过 C++ UNPRMaterialUtils::ApplyNPRMaterialToMonster 应用 NPR 材质
5. 等待材质编译
6. HighResShot 截图
7. 退出
"""
import unreal
import time
import sys

# 怪物配置：Mesh 路径 + AnimBP + 调色板索引
MONSTERS = [
    {"key": "CandyZombie",
     "mesh": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
     "anim": "/Game/Monsters/CandyZombie/ABP/ABP_CandyZombie",
     "palette": 0,
     "pos": unreal.Vector(-300, 0, 50)},
    {"key": "Gingerbread",
     "mesh": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
     "anim": "/Game/Monsters/Gingerbread/ABP/ABP_Gingerbread",
     "palette": 1,
     "pos": unreal.Vector(-100, 0, 50)},
    {"key": "ShadowNinja",
     "mesh": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
     "anim": "/Game/Monsters/ShadowNinja/ABP/ABP_ShadowNinja",
     "palette": 2,
     "pos": unreal.Vector(100, 0, 50)},
    {"key": "ArmoredGum",
     "mesh": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
     "anim": "/Game/Monsters/ArmoredGum/ABP/ABP_ArmoredGum",
     "palette": 3,
     "pos": unreal.Vector(300, 0, 50)},
]


def spawn_monster(cfg):
    mesh = unreal.load_asset(cfg["mesh"])
    if mesh is None:
        unreal.log_error(f"  ❌ Mesh 未找到: {cfg['mesh']}")
        return None

    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkeletalMeshActor, cfg["pos"], unreal.Rotator(0, 0, 0))
    if not actor:
        return None

    skel_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    if not skel_comp:
        return None

    skel_comp.set_skeletal_mesh(mesh)

    anim_bp = unreal.load_asset(cfg["anim"])
    if anim_bp:
        skel_comp.set_anim_instance_class(anim_bp.generated_class())
        unreal.log(f"  ✅ AnimBP 设置成功: {cfg['key']}")

    return actor


def apply_npr_material(actor, palette_idx):
    """通过 C++ UNPRMaterialUtils 应用 NPR 材质"""
    try:
        unreal.NPRMaterialUtils.apply_npr_material_to_monster(actor, palette_idx)
        unreal.log(f"  ✅ NPR 材质已应用 (palette={palette_idx}): {actor.get_name()}")
        return True
    except Exception as e:
        unreal.log_error(f"  ❌ NPR 材质应用失败: {e}")
        return False


def main():
    unreal.log("=" * 60)
    unreal.log("[NPR_TEST] 开始 NPR 材质验证测试")
    unreal.log("=" * 60)

    # 1) 加载测试关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    loaded = unreal.EditorLevelLibrary.load_level(level_path)
    if not loaded:
        unreal.log_error(f"  ❌ 无法加载关卡: {level_path}")
        unreal.SystemLibrary.execute_console_command(None, "quit")
        return

    unreal.log(f"  ✅ 已加载关卡: {level_path}")

    # 2) 生成怪物
    actors = []
    for cfg in MONSTERS:
        actor = spawn_monster(cfg)
        if actor:
            actors.append((actor, cfg["palette"]))
            unreal.log(f"  ✅ 生成: {cfg['key']}")

    if not actors:
        unreal.log_error("  ❌ 没有怪物被成功生成")
        unreal.SystemLibrary.execute_console_command(None, "quit")
        return

    unreal.log(f"  ✅ 共生成 {len(actors)} 只怪物")

    # 3) 添加灯光（降低强度避免过曝）
    unreal.log("  添加灯光...")
    dir_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(-45, 45, 0))
    if dir_light:
        comp = dir_light.get_component_by_class(unreal.DirectionalLightComponent)
        if comp:
            comp.set_editor_property('Intensity', 1000.0)  # 降低从 10000 → 1000
            comp.set_editor_property('LightColor', unreal.Color(r=255, g=242, b=229, a=255))
        unreal.log("  ✅ DirectionalLight 已添加 (Intensity=1000)")

    sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight, unreal.Vector(0, 0, 1000), unreal.Rotator(0, 0, 0))
    if sky_light:
        sky_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
        if sky_comp:
            sky_comp.set_editor_property('Intensity', 0.3)  # 降低天空光
        unreal.log("  ✅ SkyLight 已添加 (Intensity=0.3)")

    time.sleep(2)

    # 4) 应用 NPR 材质（通过 C++ 函数）
    success = 0
    for actor, palette in actors:
        if apply_npr_material(actor, palette):
            success += 1

    unreal.log(f"  ✅ NPR 材质应用: {success}/{len(actors)}")

    # 5) 等待渲染更新和 Shader 编译
    unreal.log("  等待渲染更新...")
    time.sleep(20)

    # 6) 截图（HighResShot 是异步的，编辑器会在空闲时处理）
    unreal.log(" 开始截图...")
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    unreal.log("  HighResShot 已提交，不主动 quit，由 timeout 终止编辑器")

    # 不再调用 quit — 让编辑器有时间处理截图
    # bash 的 timeout 会在指定时间后终止进程


if __name__ == "__main__":
    main()
else:
    main()
