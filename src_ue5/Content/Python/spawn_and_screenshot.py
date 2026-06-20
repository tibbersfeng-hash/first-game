"""
Spawn monsters in the test level and take a screenshot.
Triggered via FIRSTGAME_SPAWN_AND_SHOT=1 in init_unreal.py.
"""
import unreal
import time

MONSTERS = [
    {"key": "CandyZombie", "mesh_path": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
     "pos": unreal.Vector(-300, 0, 50)},
    {"key": "Gingerbread", "mesh_path": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
     "pos": unreal.Vector(-100, 0, 50)},
    {"key": "ShadowNinja", "mesh_path": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
     "pos": unreal.Vector(100, 0, 50)},
    {"key": "ArmoredGum", "mesh_path": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
     "pos": unreal.Vector(300, 0, 50)},
]


def main():
    unreal.log("=" * 60)
    unreal.log("[SPAWN+SHOT] Loading test level and spawning monsters")
    unreal.log("=" * 60)

    # Load the test level
    level_path = "/Game/Maps/TestLevel_Monsters"
    loaded = unreal.EditorLevelLibrary.load_level(level_path)
    if not loaded:
        unreal.log_error(f"  ❌ Failed to load level: {level_path}")
        unreal.SystemLibrary.execute_console_command(None, 'quit')
        return

    unreal.log(f"  ✅ Loaded level: {level_path}")

    # Spawn monsters
    spawned = 0
    for monster in MONSTERS:
        key = monster["key"]
        mesh_path = monster["mesh_path"]
        pos = monster["pos"]

        mesh = unreal.load_asset(mesh_path)
        if mesh is None:
            unreal.log_error(f"  ❌ Mesh not found: {mesh_path}")
            continue

        try:
            # Try spawning a SkeletalMeshActor
            actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.SkeletalMeshActor,
                pos,
                unreal.Rotator(0, 0, 0),
            )
            if actor:
                skel_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
                if skel_comp:
                    skel_comp.set_skeletal_mesh(mesh)
                    unreal.log(f"  ✅ Spawned {key} at {pos}")
                    spawned += 1
                else:
                    unreal.log_error(f"  ❌ No SkeletalMeshComponent on {key}")
        except Exception as e:
            unreal.log_error(f"  ❌ Spawn {key}: {e}")

    unreal.log(f"\n  Spawned {spawned}/{len(MONSTERS)} monsters")

    # Add a directional light if none exists
    try:
        light = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.DirectionalLight,
            unreal.Vector(0, 0, 500),
            unreal.Rotator(-45, 45, 0),
        )
        if light:
            unreal.log("  ✅ Added directional light")
    except Exception as e:
        unreal.log(f"  ⚠️  Light: {e}")

    # Save level
    unreal.EditorLevelLibrary.save_current_level()

    # Wait a moment for rendering
    unreal.log("  Waiting for render...")
    import time
    time.sleep(3)

    # Take screenshot
    unreal.log("  Taking screenshot...")
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")

    unreal.log("=" * 60)
    unreal.log(f"[SPAWN+SHOT] Complete. Check Saved/Screenshots/ for output.")
    unreal.log("=" * 60)

    unreal.SystemLibrary.execute_console_command(None, 'quit')


main()
