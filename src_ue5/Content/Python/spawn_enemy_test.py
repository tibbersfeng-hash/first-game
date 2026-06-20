import unreal
import time

def main():
    unreal.log("=" * 60)
    unreal.log("[ENEMY TEST] Spawning ABaseEnemy instances")
    unreal.log("=" * 60)

    # Load test level
    level_path = "/Game/Maps/TestLevel_Monsters"
    unreal.EditorLevelLibrary.load_level(level_path)
    unreal.log(f"  ✅ Loaded {level_path}")

    # Get ABaseEnemy class
    enemy_class = unreal.load_class(None, "/Script/FirstGame.BaseEnemy")
    if not enemy_class:
        unreal.log_error("  ❌ Failed to load ABaseEnemy class")
        return

    unreal.log(f"  ✅ Loaded ABaseEnemy class")

    # Spawn 4 enemies with different types
    positions = [(-300, 0, 50), (-100, 0, 50), (100, 0, 50), (300, 0, 50)]
    enemy_types = ["CandyZombie", "Gingerbread", "ShadowNinja", "ArmoredGum"]

    for i, (pos, etype) in enumerate(zip(positions, enemy_types)):
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            enemy_class,
            unreal.Vector(pos[0], pos[1], pos[2]),
            unreal.Rotator(0, 0, 0),
        )
        if actor:
            # Set EnemyType property
            try:
                type_map = {"CandyZombie": 0, "Gingerbread": 1, "ShadowNinja": 2, "ArmoredGum": 3}
                actor.set_editor_property('EnemyType', type_map[etype])
                unreal.log(f"  ✅ Spawned {etype} at {pos} (EnemyType={type_map[etype]})")
            except Exception as e:
                unreal.log(f"  ⚠️  {etype}: {e}")
        else:
            unreal.log_error(f"  ❌ Failed to spawn {etype}")

    # Add lights
    try:
        dl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.DirectionalLight,
            unreal.Vector(0, -500, 500),
            unreal.Rotator(-60, 45, 0),
        )
        unreal.log("  ✅ Directional light")
    except:
        pass

    try:
        sl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.SkyLight,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0),
        )
        unreal.log("  ✅ Sky light")
    except:
        pass

    # Save level
    unreal.EditorLevelLibrary.save_current_level()

    # Wait for rendering
    time.sleep(5)

    # Screenshot
    unreal.log("  Taking screenshot...")
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")

    unreal.log("=" * 60)
    unreal.log("[ENEMY TEST] Complete")
    unreal.log("=" * 60)

    unreal.SystemLibrary.execute_console_command(None, 'quit')

main()
