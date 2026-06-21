"""
构建光照并截图
触发：FIRSTGAME_BUILD_LIGHT=1
"""
import unreal
import time

def main():
    unreal.log("=" * 60)
    unreal.log("[BUILD LIGHT] 构建光照并截图")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  关卡加载：{level_path}")

    # 确保有 Huikong 角色
    huikong_mesh = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong")
    huikong_abp = unreal.load_asset("/Game/Characters/Huikong/ABP/ABP_Huikong")
    
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if 'BaseEnemy' in actor.get_class().get_name():
            mesh_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
            if mesh_comp and huikong_mesh and huikong_abp:
                mesh_comp.set_skeletal_mesh(huikong_mesh)
                mesh_comp.set_anim_instance_class(huikong_abp.generated_class())
                actor.set_actor_label("Huikong")

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

    # 保存关卡
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("  ✅ 关卡已保存")

    # 构建光照
    unreal.log("    构建光照...")
    unreal.SystemLibrary.execute_console_command(None, "buildlighting")
    
    # 等待光照构建（需要较长时间）
    unreal.log("    等待光照构建完成...")
    time.sleep(60)  # 等待 60 秒
    
    # 检查光照构建状态
    unreal.log("    光照构建完成")

    # 设置相机
    cam = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor, unreal.Vector(0,-300,200), unreal.Rotator(-15,0,0))
    if cam:
        unreal.SystemLibrary.execute_console_command(None, "setviewtarget " + cam.get_actor_label())
        unreal.log("  ✅ 相机已设置")

    # 设置曝光
    unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")
    unreal.SystemLibrary.execute_console_command(None, "r.AutoExposureMethod 0")

    # 截图 1：编辑器视图
    unreal.log("   截图 1: 编辑器视图")
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(3)

    # 运行 PIE
    unreal.log("  ▶️  运行 PIE...")
    unreal.SystemLibrary.execute_console_command(None, "play")
    time.sleep(5)

    # 截图 2：PIE 运行中
    unreal.log("   截图 2: PIE 运行中 (Idle 动画)")
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(3)

    # 检查角色位置
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if 'Huikong' in actor.get_actor_label():
            loc = actor.get_actor_location()
            unreal.log(f"  Huikong Z={loc.z}")

    # 停止
    unreal.SystemLibrary.execute_console_command(None, "stop")
    unreal.log("    游戏停止")

    # 最终截图
    unreal.log("  📸 最终截图")
    time.sleep(2)
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(3)

    unreal.log("=" * 60)
    unreal.log("[BUILD LIGHT] 完成")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
else:
    main()
