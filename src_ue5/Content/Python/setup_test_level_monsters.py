"""
Create a test level with all 4 monsters placed for visual verification.
Triggered via FIRSTGAME_TEST_LEVEL=1 in init_unreal.py.
"""
import unreal

MONSTERS = [
    {"key": "CandyZombie", "mesh_path": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
     "abp_path": "/Game/Monsters/CandyZombie/ABP/ABP_CandyZombie", "pos": (-400, 0, 0)},
    {"key": "Gingerbread", "mesh_path": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
     "abp_path": "/Game/Monsters/Gingerbread/ABP/ABP_Gingerbread", "pos": (-200, 0, 0)},
    {"key": "ShadowNinja", "mesh_path": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
     "abp_path": "/Game/Monsters/ShadowNinja/ABP/ABP_ShadowNinja", "pos": (0, 0, 0)},
    {"key": "ArmoredGum", "mesh_path": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
     "abp_path": "/Game/Monsters/ArmoredGum/ABP/ABP_ArmoredGum", "pos": (200, 0, 0)},
]


def main():
    unreal.log("=" * 60)
    unreal.log("[TEST LEVEL] Creating Monster Test Level")
    unreal.log("=" * 60)

    # Create a new level
    map_path = "/Game/Maps/TestLevel_Monsters"
    if unreal.EditorAssetLibrary.does_asset_exist(map_path):
        unreal.log(f"  Map already exists: {map_path}")
        # Load it
        unreal.EditorLevelLibrary.load_level(map_path)
    else:
        # Create new empty level
        success = unreal.EditorLevelLibrary.new_level(map_path)
        if not success:
            unreal.log_error(f"  ❌ Failed to create level: {map_path}")
            unreal.SystemLibrary.execute_console_command(None, 'quit')
            return
        unreal.log(f"  ✅ Created level: {map_path}")

    # Add a floor plane
    try:
        floor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(0, 0, -50),
            unreal.Rotator(0, 0, 0),
        )
        if floor:
            # Use default plane mesh
            floor_mesh = unreal.load_asset("/Engine/BasicShapes/Plane")
            if floor_mesh:
                floor.set_editor_property('static_mesh', floor_mesh)
                floor.set_actor_scale3d(unreal.Vector(5, 5, 1))
            unreal.log("  ✅ Added floor")
    except Exception as e:
        unreal.log(f"  ⚠️  Floor: {e}")

    # Add directional light
    try:
        light = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.DirectionalLightActor,
            unreal.Vector(0, 0, 500),
            unreal.Rotator(-45, 45, 0),
        )
        if light:
            unreal.log("  ✅ Added directional light")
    except Exception as e:
        unreal.log(f"  ⚠️  Light: {e}")

    # Place monsters
    placed = 0
    for monster in MONSTERS:
        key = monster["key"]
        mesh_path = monster["mesh_path"]
        pos = monster["pos"]

        mesh = unreal.load_asset(mesh_path)
        if mesh is None:
            unreal.log_error(f"  ❌ Mesh not found: {mesh_path}")
            continue

        # Spawn SkeletalMesh actor
        try:
            actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.SkeletalMeshActor,
                unreal.Vector(pos[0], pos[1], 50),
                unreal.Rotator(0, 0, 0),
            )
            if actor:
                skeletal_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
                if skeletal_comp:
                    skeletal_comp.set_skeletal_mesh(mesh)
                    # Set AnimBP class
                    abp = unreal.load_asset(monster["abp_path"])
                    if abp:
                        skeletal_comp.set_anim_instance_class(abp.generated_class())
                        unreal.log(f"  ✅ Placed {key} with AnimBP")
                    else:
                        unreal.log(f"  ✅ Placed {key} (no AnimBP)")
                    placed += 1
                else:
                    unreal.log_error(f"  ❌ No SkeletalMeshComponent on {key}")
        except Exception as e:
            unreal.log_error(f"  ❌ Place {key}: {e}")

    # Save the level
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log(f"\n  Placed {placed}/4 monsters")

    unreal.log("=" * 60)
    unreal.log(f"[TEST LEVEL] Complete: {map_path}")
    unreal.log("=" * 60)

    unreal.SystemLibrary.execute_console_command(None, 'quit')


main()
