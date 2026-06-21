"""
验证飞天问题修复 - 运行游戏 15 秒，每 3 秒截图一次
触发：FIRSTGAME_VERIFY_FLYING=1
"""
import unreal
import time

def main():
    unreal.log("=" * 60)
    unreal.log("[VERIFY FLYING] 验证飞天问题修复")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  关卡加载：{level_path}")
    time.sleep(3)

    # 检查是否有地面
    has_floor = False
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        label = actor.get_actor_label()
        if 'Floor' in label or 'Plane' in label:
            has_floor = True
            unreal.log(f"  ✅ 找到地面：{label}")
            break
    
    if not has_floor:
        unreal.log("    未找到地面，添加地面...")
        # 使用 Cube 代替 Plane（更可靠）
        floor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0),
        )
        if floor:
            cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube")
            if cube_mesh:
                # 获取 StaticMeshComponent 并设置网格体
                mesh_comp = floor.get_component_by_class(unreal.StaticMeshComponent)
                if mesh_comp:
                    mesh_comp.set_static_mesh(cube_mesh)
                    floor.set_actor_scale3d(unreal.Vector(10, 10, 0.1))
                    floor.set_actor_label("Floor")
                    unreal.log("    ✅ 地面已添加 (Cube, Scale=10x10x0.1)")

    # 找到所有 BaseEnemy
    enemies = []
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        cls_name = actor.get_class().get_name()
        if 'BaseEnemy' in cls_name:
            enemies.append(actor)
            loc = actor.get_actor_location()
            unreal.log(f"  找到敌人：{actor.get_actor_label()} at Z={loc.z}")

    unreal.log(f"  共 {len(enemies)} 个敌人")

    if len(enemies) == 0:
        unreal.log_error("  没有找到 BaseEnemy！")
        return

    # 记录初始位置
    initial_z = {}
    for enemy in enemies:
        loc = enemy.get_actor_location()
        initial_z[enemy.get_actor_label()] = loc.z

    # 设置相机（俯视角度）
    cam = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor, unreal.Vector(0, -800, 600), unreal.Rotator(-20, 0, 0))
    if cam:
        unreal.SystemLibrary.execute_console_command(None, "setviewtarget " + cam.get_actor_label())
        unreal.log("    相机设置完成")

    # 截图 1：初始状态
    unreal.log("    截图 1: 初始状态")
    unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")
    time.sleep(1)
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(3)

    # 运行游戏
    unreal.log("    开始运行游戏 15 秒...")
    unreal.SystemLibrary.execute_console_command(None, "play")

    # 每 3 秒截图并检查位置
    for second in range(1, 6):
        time.sleep(3)
        unreal.log(f"    --- {second*3} 秒后 ---")
        
        flying_count = 0
        for enemy in enemies:
            loc = enemy.get_actor_location()
            name = enemy.get_actor_label()
            delta_z = loc.z - initial_z.get(name, 50)
            
            if loc.z > 200:  # Z > 200 算飞天
                flying_count += 1
                unreal.log(f"      警告：{name} 飞天！Z={loc.z} (上升 {delta_z:.1f})")
            else:
                unreal.log(f"      正常：{name} Z={loc.z}")

        # 截图
        unreal.log(f"    截图 {second+1}: {second*3}秒")
        unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
        time.sleep(2)

        if flying_count > 0:
            unreal.log(f"    警告：{flying_count}/{len(enemies)} 个敌人飞天")
        else:
            unreal.log(f"    成功：所有敌人正常")

    # 停止游戏
    unreal.SystemLibrary.execute_console_command(None, "stop")
    unreal.log("    游戏停止")

    # 最终截图
    unreal.log("    最终截图")
    time.sleep(2)
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(3)

    unreal.log("=" * 60)
    unreal.log("[VERIFY FLYING] 验证完成")
    unreal.log("=" * 60)

    # 不退出编辑器，让用户查看
    unreal.log("  编辑器保持运行，请查看截图")


if __name__ == "__main__":
    main()
else:
    main()
