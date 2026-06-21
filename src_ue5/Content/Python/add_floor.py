"""
添加地面到 TestLevel_Monsters 关卡
触发：FIRSTGAME_ADD_FLOOR=1
"""
import unreal

def main():
    unreal.log("=" * 60)
    unreal.log("[ADD FLOOR] 添加地面到 TestLevel_Monsters")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  关卡加载：{level_path}")

    # 添加地面 (Plane)
    floor_pos = unreal.Vector(0, 0, -50)
    floor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        floor_pos,
        unreal.Rotator(0, 0, 0),
    )

    if floor:
        # 设置网格体
        floor_mesh = unreal.load_asset("/Engine/BasicShapes/Plane")
        if floor_mesh:
            floor.set_editor_property('static_mesh', floor_mesh)
            floor.set_actor_scale3d(unreal.Vector(5, 5, 1))
            floor.set_actor_label("Floor")
            unreal.log("  ✅ 添加地面 (Plane, Scale=5)")

        # 设置碰撞和材质
        static_mesh_comp = floor.get_component_by_class(unreal.StaticMeshComponent)
        if static_mesh_comp:
            # 启用碰撞
            static_mesh_comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
            # 设置碰撞预设
            static_mesh_comp.set_collision_profile_name("BlockAll")
            unreal.log("  ✅ 碰撞设置完成")

    # 保存关卡
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("  ✅ 关卡已保存")

    unreal.log("=" * 60)
    unreal.log("[ADD FLOOR] 完成")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
else:
    main()
