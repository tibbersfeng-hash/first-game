"""
Setup Test Level - Idempotent (v4)
====================================
Only creates AnimTestActor if it doesn't already exist.
"""
import unreal

LOG = "[SETUP]"
def log(msg):
    unreal.log(f"{LOG} {msg}")
def log_error(msg):
    unreal.log_error(f"{LOG} {msg}")

MESH = "/Game/Characters/Huikong/Mesh/SKM_Huikong"
IDLE = "/Game/Characters/Huikong/Animations/AM_Huikong_Idle_01"
WALK = "/Game/Characters/Huikong/Animations/AM_Huikong_Walk_01"
RUN  = "/Game/Characters/Huikong/Animations/AM_Huikong_Run_01"
MAP  = "/Game/Maps/TestLevel_AnimTest"

log("=== Setup v4 (idempotent) ===")

editor_level = unreal.EditorLevelLibrary
world = editor_level.get_editor_world()
actors = editor_level.get_all_level_actors()

log(f"World: {world.get_name()}, Actors: {len(actors)}")

# Check if AnimTestActor already exists
has_anim_actor = False
for a in actors:
    if "AnimTestActor" in a.get_name():
        has_anim_actor = True
        log(f"AnimTestActor already exists: {a.get_name()}")
        break

if has_anim_actor:
    log("=== Already set up, skipping ===")
else:
    # Create level only if it's the right map
    if world.get_name() != "TestLevel_AnimTest":
        log("Wrong world, loading TestLevel_AnimTest...")
        if unreal.EditorAssetLibrary.does_asset_exist(MAP):
            editor_level.load_level(MAP)
            world = editor_level.get_editor_world()
        else:
            editor_level.new_level(MAP)
            world = editor_level.get_editor_world()

    actors = editor_level.get_all_level_actors()
    log(f"Actors in level: {len(actors)}")

    # Spawn AnimTestActor
    cls = unreal.load_class(None, "/Script/FirstGame.AnimTestActor")
    actor = editor_level.spawn_actor_from_class(cls, unreal.Vector(0,0,0), unreal.Rotator(0,0,0))
    log(f"Spawned: {actor.get_name()}")

    # Set mesh
    mesh_comp = actor.get_editor_property("SkeletalMesh")
    mesh_asset = unreal.load_asset(MESH)
    if mesh_comp and mesh_asset:
        mesh_comp.set_editor_property("SkeletalMesh", mesh_asset)
        log("SkeletalMesh set")

    # Set animations
    actor.set_editor_property("IdleAnim", unreal.load_asset(IDLE))
    actor.set_editor_property("WalkAnim", unreal.load_asset(WALK))
    actor.set_editor_property("RunAnim", unreal.load_asset(RUN))
    log("Animations set")

    # Playback
    actor.set_editor_property("bAutoPlay", True)
    actor.set_editor_property("SwitchInterval", 3.0)
    actor.set_editor_property("BlendTime", 0.2)
    log("AutoPlay=True, SwitchInterval=3.0")

    # Save
    actor.modify()
    editor_level.save_current_level()
    log(f"Saved. Total actors: {len(editor_level.get_all_level_actors())}")

log("=== Done ===")

# Auto-start PIE after a short delay to let editor fully load
import threading
def start_pie():
    import time
    time.sleep(5)  # Wait for editor to be fully ready
    try:
        # Method 1: Console command
        unreal.SystemLibrary.execute_console_command(world, "PIE")
        log("PIE started via console command")
    except Exception as e1:
        log(f"Console command failed: {e1}")
        try:
            # Method 2: Try EditorLevelLibrary
            editor_level.start_play_in_editor()
            log("PIE started via EditorLevelLibrary")
        except Exception as e2:
            log_error(f"PIE start failed: {e2}")

threading.Thread(target=start_pie, daemon=True).start()
log("PIE will auto-start in 5 seconds...")
