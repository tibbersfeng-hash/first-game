"""
设置 Huikong 测试角色
触发：FIRSTGAME_SETUP_HUIKONG=1
"""
import unreal

def main():
    unreal.log("=" * 60)
    unreal.log("[SETUP HUIKONG] 设置 Huikong 测试角色")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  关卡加载：{level_path}")

    # 删除所有现有 BaseEnemy
    enemies_to_delete = []
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if 'BaseEnemy' in actor.get_class().get_name():
            enemies_to_delete.append(actor)
            unreal.log(f"  删除：{actor.get_actor_label()}")
    
    for enemy in enemies_to_delete:
        unreal.EditorLevelLibrary.destroy_actor(enemy)

    # 添加地面
    has_floor = False
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if 'Floor' in actor.get_actor_label():
            has_floor = True
            break
    
    if not has_floor:
        floor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )
        if floor:
            floor.set_actor_label("Floor")
            mesh = unreal.load_asset("/Engine/BasicShapes/Cube")
            comp = floor.get_component_by_class(unreal.StaticMeshComponent)
            if comp:
                comp.set_static_mesh(mesh)
            floor.set_actor_scale3d(unreal.Vector(10, 10, 0.1))
            unreal.log("  ✅ 地面已添加")

    # 添加 Huikong 角色（使用 BaseEnemy + Huikong 资产）
    huikong = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.EditorLevelLibrary.get_editor_world().get_actor_class(),  # 使用默认类
        unreal.Vector(0, 0, 50),  # Z=50 站在地面上
        unreal.Rotator(0, 0, 0)
    )
    
    # 实际上我们需要使用 BaseEnemy 类，但设置 Huikong 的网格体和 AnimBP
    # 先加载 Huikong 资产
    huikong_mesh = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong")
    huikong_abp = unreal.load_asset("/Game/Characters/Huikong/ABP/ABP_Huikong")
    
    if huikong_mesh and huikong_abp:
        unreal.log("  ✅ Huikong 资产加载成功")
        unreal.log(f"     Mesh: {huikong_mesh.get_name()}")
        unreal.log(f"     AnimBP: {huikong_abp.get_name()}")
    else:
        unreal.log_error("  ❌ Huikong 资产加载失败")
        return

    # 保存关卡
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("  ✅ 关卡已保存")

    unreal.log("=" * 60)
    unreal.log("[SETUP HUIKONG] 完成")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
else:
    main()
