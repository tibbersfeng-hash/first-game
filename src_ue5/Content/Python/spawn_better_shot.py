"""
Spawn monsters with better lighting and camera for screenshot.
Triggered via FIRSTGAME_BETTER_SHOT=1 in init_unreal.py.
"""
import unreal
import time


def main():
    unreal.log("=" * 60)
    unreal.log("[BETTER SHOT] Better lighting + screenshot")
    unreal.log("=" * 60)

    # Load level
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  ✅ Loaded {level_path}")

    # Spawn monsters with colored dynamic materials
    MONSTERS = [
        {"key": "CandyZombie", "mesh": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
         "color": (0.2, 0.8, 0.3)},
        {"key": "Gingerbread", "mesh": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
         "color": (0.65, 0.4, 0.2)},
        {"key": "ShadowNinja", "mesh": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
         "color": (0.5, 0.3, 0.8)},
        {"key": "ArmoredGum", "mesh": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
         "color": (0.75, 0.78, 0.8)},
    ]

    spawned = 0
    for i, m in enumerate(MONSTERS):
        mesh = unreal.load_asset(m["mesh"])
        if mesh is None:
            continue

        try:
            pos = unreal.Vector(-200 + i * 150, 0, 60)
            actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.SkeletalMeshActor, pos, unreal.Rotator(0, 0, 0)
            )
            if actor:
                skel = actor.get_component_by_class(unreal.SkeletalMeshComponent)
                if skel:
                    skel.set_skeletal_mesh(mesh)
                    # Try to create dynamic material with color
                    try:
                        materials = mesh.get_materials()
                        if len(materials) > 0:
                            base_mat = materials[0].material_interface
                            if base_mat:
                                dyn_mat = unreal.MaterialInstanceDynamic.create(base_mat, None)
                                if dyn_mat:
                                    color = unreal.LinearColor(m["color"][0], m["color"][1], m["color"][2], 1.0)
                                    try:
                                        dyn_mat.set_vector_parameter_value('Base Color', color)
                                    except:
                                        try:
                                            dyn_mat.set_vector_parameter_value('BaseColor', color)
                                        except:
                                            pass
                                    skel.set_material(0, dyn_mat)
                                    unreal.log(f"  ✅ {m['key']} with colored material")
                    except Exception as e:
                        unreal.log(f"  ⚠️  Material for {m['key']}: {e}")
                    spawned += 1
                    unreal.log(f"  ✅ Spawned {m['key']} at {pos}")
        except Exception as e:
            unreal.log(f"  ❌ {m['key']}: {e}")

    unreal.log(f"\n  Spawned {spawned}/{len(MONSTERS)}")

    # Add bright point lights near each monster
    for i in range(spawned):
        pos = unreal.Vector(-200 + i * 150, 100, 200)
        try:
            pl = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.PointLight, pos, unreal.Rotator(0, 0, 0)
            )
            if pl:
                unreal.log(f"  ✅ Point light at {pos}")
        except:
            pass

    # Add bright directional light
    try:
        dl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.DirectionalLight,
            unreal.Vector(0, -500, 500),
            unreal.Rotator(-60, 45, 0),
        )
        if dl:
            unreal.log("  ✅ Directional light")
    except:
        pass

    # Add sky light
    try:
        sl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.SkyLight, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0)
        )
        if sl:
            unreal.log("  ✅ Sky light")
    except:
        pass

    unreal.EditorLevelLibrary.save_current_level()

    # Wait for rendering
    time.sleep(5)

    # Screenshot
    unreal.log("  Taking screenshot...")
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")

    unreal.log("=" * 60)
    unreal.log("[BETTER SHOT] Complete")
    unreal.log("=" * 60)

    unreal.SystemLibrary.execute_console_command(None, 'quit')


main()
