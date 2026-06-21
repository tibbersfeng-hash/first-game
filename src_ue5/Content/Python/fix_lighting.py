"""
修复关卡光照问题
触发：FIRSTGAME_FIX_LIGHTING=1
"""
import unreal

def main():
    unreal.log("=" * 60)
    unreal.log("[FIX LIGHTING] 修复关卡光照")
    unreal.log("=" * 60)

    # 加载关卡
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  关卡加载：{level_path}")

    # 删除所有现有光源
    lights_to_delete = []
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        cls_name = actor.get_class().get_name()
        if 'Light' in cls_name:
            lights_to_delete.append(actor)
            unreal.log(f"  删除光源：{actor.get_actor_label()} ({cls_name})")
    
    for light in lights_to_delete:
        unreal.EditorLevelLibrary.destroy_actor(light)

    # 添加主方向光（使用正确的类名）
    sun_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 1000),
        unreal.Rotator(-45, 45, 0)
    )
    if sun_light:
        sun_light.set_actor_label("SunLight")
        light_comp = sun_light.get_component_by_class(unreal.DirectionalLightComponent)
        if light_comp:
            light_comp.set_editor_property('intensity', 100000.0)
            light_comp.set_editor_property('light_color', unreal.LinearColor(1.0, 0.95, 0.9, 1.0))
        unreal.log("  ✅ 主方向光已添加")

    # 添加天光
    sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 500),
        unreal.Rotator(0, 0, 0)
    )
    if sky_light:
        sky_light.set_actor_label("SkyLight")
        unreal.log("  ✅ 天光已添加")

    # 添加地面（如果还没有）
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

    # 保存关卡
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("  ✅ 关卡已保存")

    unreal.log("=" * 60)
    unreal.log("[FIX LIGHTING] 完成 - 请查看编辑器")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
else:
    main()
