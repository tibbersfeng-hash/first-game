"""
测试角色飞天问题修复
触发：FIRSTGAME_TEST_FLYING=1
"""
import unreal
import time

def main():
    unreal.log("=" * 60)
    unreal.log("[TEST FLYING] 测试角色飞天问题修复")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.log(f"  加载关卡：{level_path}")
    loaded = unreal.EditorLevelLibrary.load_level(level_path)
    if not loaded:
        unreal.log_error("  关卡加载失败")
        return
    unreal.log("  关卡加载成功")

    # 等待关卡加载
    time.sleep(3)

    # 获取所有 Actor
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    unreal.log(f"  关卡中有 {len(all_actors)} 个 Actor")

    # 查找角色/敌人
    characters = []
    for actor in all_actors:
        cls_name = actor.get_class().get_name()
        if 'Enemy' in cls_name or 'Character' in cls_name or 'SkeletalMesh' in cls_name:
            characters.append(actor)
            loc = actor.get_actor_location()
            unreal.log(f"  找到：{cls_name} at {loc}")

    unreal.log(f"  共找到 {len(characters)} 个角色/敌人")

    # 设置相机
    cam_pos = unreal.Vector(0, -500, 300)
    cam_rot = unreal.Rotator(-20, 0, 0)
    cam = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CameraActor, cam_pos, cam_rot)
    if cam:
        unreal.SystemLibrary.execute_console_command(
            None, "setviewtarget " + cam.get_actor_label())
        unreal.log("  相机设置完成")

    # 等待几帧让物理生效
    unreal.log("  等待物理生效...")
    time.sleep(5)

    # 检查角色位置
    unreal.log("  检查角色位置...")
    for actor in characters[:5]:  # 只检查前 5 个
        loc = actor.get_actor_location()
        cls_name = actor.get_class().get_name()
        # Z > 100 表示飞天
        if loc.z > 100:
            unreal.log(f"  ⚠️  {cls_name} 飞天！Z={loc.z}")
        else:
            unreal.log(f"  ✅ {cls_name} 正常 Z={loc.z}")

    # 截图
    unreal.log("  截图...")
    unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")
    unreal.SystemLibrary.execute_console_command(None, "r.AutoExposureMethod 0")
    time.sleep(2)
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(5)

    unreal.log("=" * 60)
    unreal.log("[TEST FLYING] 测试完成")
    unreal.log("=" * 60)

    # 不退出编辑器，让用户查看
    unreal.log("  编辑器保持运行，请手动查看结果")


if __name__ == "__main__":
    main()
else:
    main()
