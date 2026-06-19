"""
Setup Clean Test Level - AnimTestActor only
=============================================
Creates a completely new empty level with only AnimTestActor.
Also fixes Animation Mode to "Use Animation Asset".
"""
import unreal

LOG = "[SETUP]"

def log(msg):
    unreal.log(f"{LOG} {msg}")

def log_error(msg):
    unreal.log_error(f"{LOG} {msg}")

# ─── Paths ─────────────────────────────────────────────────────
MESH_PATH = "/Game/Characters/Huikong/Mesh/SKM_Huikong"
IDLE_PATH = "/Game/Characters/Huikong/Animations/AM_Huikong_Idle_01"
WALK_PATH = "/Game/Characters/Huikong/Animations/AM_Huikong_Walk_01"
RUN_PATH = "/Game/Characters/Huikong/Animations/AM_Huikong_Run_01"
MAP_PATH = "/Game/Maps/TestLevel_AnimTest"

log("=== Clean Test Level Setup ===")

# ─── Delete existing map if any ─────────────────────────────────
if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    log("Deleting existing map...")
    unreal.EditorAssetLibrary.delete_asset(MAP_PATH)
    unreal.EditorAssetLibrary.save_deleted_assets()

# ─── Create NEW empty level ─────────────────────────────────────
log("Creating new empty level...")
editor_level = unreal.EditorLevelLibrary

try:
    # new_level creates a completely empty level
    result = editor_level.new_level(MAP_PATH)
    log(f"Level created: {result}")
except Exception as e:
    log_error(f"Failed to create level: {e}")
    import traceback
    log_error(traceback.format_exc())
    exit(1)

# Get the current world
world = editor_level.get_editor_world()
log(f"World: {world.get_name()}")

# ─── Verify world is empty ─────────────────────────────────────
actors = editor_level.get_all_level_actors()
log(f"Actors in level: {len(actors)} (should be 0)")

# ─── Spawn AnimTestActor ───────────────────────────────────────
log("Spawning AnimTestActor...")

actor_class = unreal.load_class(None, "/Script/FirstGame.AnimTestActor")
if not actor_class:
    log_error("AnimTestActor class not found!")
    exit(1)

actor = editor_level.spawn_actor_from_class(
    actor_class,
    unreal.Vector(0, 0, 0),
    unreal.Rotator(0, 0, 0)
)

if not actor:
    log_error("Failed to spawn!")
    exit(1)

log(f"Spawned: {actor.get_name()}")

# ── Configure properties ──────────────────────────────────────
log("Configuring properties...")

# 1. Set SkeletalMesh component
try:
    mesh_comp = actor.get_editor_property("SkeletalMesh")
    mesh_asset = unreal.load_asset(MESH_PATH)
    if mesh_comp and mesh_asset:
        mesh_comp.set_editor_property("SkeletalMesh", mesh_asset)
        # CRITICAL: Set Animation Mode to "Use Animation Asset"
        # EAnimationMode::UseAnimationAsset = 1
        # EAnimationMode::UseAnimationBlueprint = 0
        mesh_comp.set_editor_property("AnimationMode", 1)
        log(f"  SkeletalMesh set, AnimationMode = UseAnimationAsset")
    else:
        log_error(f"  mesh_comp={mesh_comp}, mesh_asset={mesh_asset}")
except Exception as e:
    log_error(f"  Set mesh error: {e}")
    import traceback
    log_error(traceback.format_exc())

# 2. Set animations
try:
    idle = unreal.load_asset(IDLE_PATH)
    walk = unreal.load_asset(WALK_PATH)
    run = unreal.load_asset(RUN_PATH)
    actor.set_editor_property("IdleAnim", idle)
    actor.set_editor_property("WalkAnim", walk)
    actor.set_editor_property("RunAnim", run)
    log(f"  Animations: Idle={idle.get_name()}, Walk={walk.get_name()}, Run={run.get_name()}")
except Exception as e:
    log_error(f"  Set animations error: {e}")

# 3. Set playback settings
try:
    actor.set_editor_property("bAutoPlay", True)
    actor.set_editor_property("SwitchInterval", 3.0)
    actor.set_editor_property("BlendTime", 0.2)
    log("  AutoPlay=True, SwitchInterval=3.0, BlendTime=0.2")
except Exception as e:
    log_error(f"  Set playback error: {e}")

# ─── Verify ─────────────────────────────────────────────────────
log("Verifying...")
actors = editor_level.get_all_level_actors()
log(f"Total actors in level: {len(actors)}")
for a in actors:
    log(f"  - {a.get_name()} ({a.get_class().get_name()})")

# ─── Save ───────────────────────────────────────────────────────
log("Saving...")
try:
    actor.modify()
    editor_level.save_current_level()
    log(f"Level saved: {MAP_PATH}")
except Exception as e:
    log_error(f"Save error: {e}")
    import traceback
    log_error(traceback.format_exc())

log("=== Setup Complete ===")
log("")
log("Steps to test:")
log("1. World Outliner should show only: AnimTestActor_0")
log("2. Click it -> Details panel shows Idle/Walk/Run anims")
log("3. Animation Mode should be '使用动画资产'")
log("4. Click Play (▶) to test")
