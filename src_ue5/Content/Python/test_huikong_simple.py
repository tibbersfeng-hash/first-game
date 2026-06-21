"""
慧空动画简化测试
"""
import unreal
import time

unreal.log("=" * 60)
unreal.log("[HUIKONG SIMPLE TEST] 开始测试")
unreal.log("=" * 60)

# 加载关卡
level_path = "/Game/Maps/TestLevel_Monsters"
unreal.EditorLevelLibrary.load_level(level_path)
unreal.log(f"  关卡加载：{level_path}")
time.sleep(3)

# 设置慧空资产
huikong_mesh = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong")
huikong_abp = unreal.load_asset("/Game/Characters/Huikong/ABP/ABP_Huikong")

if huikong_mesh and huikong_abp:
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if 'BaseEnemy' in actor.get_class().get_name():
            mesh_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
            if mesh_comp:
                mesh_comp.set_skeletal_mesh(huikong_mesh)
                mesh_comp.set_anim_instance_class(huikong_abp.generated_class())
                actor.set_actor_label("Huikong")
    unreal.log("  ✅ 慧空资产已设置")

# 确保有地面
has_floor = any('Floor' in a.get_actor_label() for a in unreal.EditorLevelLibrary.get_all_level_actors())
if not has_floor:
    floor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(0,0,0), unreal.Rotator(0,0,0))
    if floor:
        floor.set_actor_label("Floor")
        mesh = unreal.load_asset("/Engine/BasicShapes/Cube")
        comp = floor.get_component_by_class(unreal.StaticMeshComponent)
        if comp:
            comp.set_static_mesh(mesh)
        floor.set_actor_scale3d(unreal.Vector(10, 10, 0.1))
        unreal.log("  ✅ 地面已添加")

# 添加光照
sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.DirectionalLight, unreal.Vector(0,0,1000), unreal.Rotator(-45,45,0))
sun.set_actor_label("Sun")
unreal.log("  ✅ 方向光已添加")

# 设置相机
cam = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.CameraActor, unreal.Vector(0,-300,200), unreal.Rotator(-15,0,0))
if cam:
    unreal.SystemLibrary.execute_console_command(None, "setviewtarget " + cam.get_actor_label())
    unreal.log("  ✅ 相机已设置")

# 设置渲染
unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")
unreal.SystemLibrary.execute_console_command(None, "r.AutoExposureMethod 0")

# 保存
unreal.EditorLevelLibrary.save_current_level()
unreal.log("  ✅ 关卡已保存")

# 截图 1: Idle
unreal.log("  📸 截图 1: Idle")
time.sleep(2)
unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
time.sleep(3)

# 运行 PIE
unreal.log("  ▶️  运行 PIE...")
unreal.SystemLibrary.execute_console_command(None, "play")
time.sleep(5)

# 截图 2: PIE
unreal.log("  📸 截图 2: PIE")
unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
time.sleep(3)

# 检查位置
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if 'Huikong' in actor.get_actor_label():
        loc = actor.get_actor_location()
        unreal.log(f"  Huikong Z={loc.z}")

# 停止
unreal.SystemLibrary.execute_console_command(None, "stop")
unreal.log("    游戏停止")

unreal.log("=" * 60)
unreal.log("[HUIKONG SIMPLE TEST] 完成")
unreal.log("=" * 60)
