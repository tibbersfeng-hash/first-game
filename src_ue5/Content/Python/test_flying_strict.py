"""
严格测试角色飞天问题 - 实际运行游戏 10 秒
触发：FIRSTGAME_TEST_FLYING_STRICT=1
"""
import unreal
import time

def main():
    unreal.log("=" * 60)
    unreal.log("[TEST FLYING STRICT] 严格测试角色飞天问题")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  关卡加载：{level_path}")
    time.sleep(3)

    # 找到所有 BaseEnemy
    enemies = []
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if 'BaseEnemy' in actor.get_class().get_name():
            enemies.append(actor)
            loc = actor.get_actor_location()
            unreal.log(f"  找到敌人：{actor.get_name()} at Z={loc.z}")

    unreal.log(f"  共 {len(enemies)} 个敌人")

    # 记录初始位置
    initial_z = {}
    for enemy in enemies:
        loc = enemy.get_actor_location()
        initial_z[enemy.get_name()] = loc.z

    # 设置相机
    cam = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor, unreal.Vector(0, -600, 400), unreal.Rotator(-15, 0, 0))
    if cam:
        unreal.SystemLibrary.execute_console_command(None, "setviewtarget " + cam.get_actor_label())

    # 运行游戏 10 秒（模拟 PIE）
    unreal.log("  开始运行游戏 10 秒...")
    unreal.SystemLibrary.execute_console_command(None, "play")

    # 每 2 秒检查一次位置
    for second in range(1, 6):
        time.sleep(2)
        unreal.log(f"  --- {second*2} 秒后 ---")
        flying_count = 0
        for enemy in enemies:
            loc = enemy.get_actor_location()
            name = enemy.get_name()
            delta_z = loc.z - initial_z.get(name, 50)
            if loc.z > 200:  # Z > 200 算飞天
                flying_count += 1
                unreal.log(f"    ⚠️  {name} 飞天！Z={loc.z} (上升 {delta_z:.1f})")
            else:
                unreal.log(f"    ✅ {name} 正常 Z={loc.z}")

        if flying_count > 0:
            unreal.log(f"  ⚠️  {flying_count}/{len(enemies)} 个敌人飞天")
        else:
            unreal.log(f"  ✅ 所有敌人正常")

    # 停止游戏
    unreal.SystemLibrary.execute_console_command(None, "stop")
    unreal.log("  游戏停止")

    # 截图
    unreal.log("  截图...")
    unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")
    time.sleep(1)
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(5)

    unreal.log("=" * 60)
    unreal.log("[TEST FLYING STRICT] 测试完成")
    unreal.log("=" * 60)

    unreal.SystemLibrary.execute_console_command(None, "quit")


if __name__ == "__main__":
    main()
else:
    main()
