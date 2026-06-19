"""
Setup Test Level with AnimTestActor
=====================================
Creates a test level and places AnimTestActor with Huikong mesh + animations.
"""
import unreal
import os

LOG = "[TEST-LVL]"

def log(msg):
    unreal.log(f"{LOG} {msg}")

def log_error(msg):
    unreal.log_error(f"{LOG} {msg}")

log("=== Creating Test Level ===")

# ─── Paths ─────────────────────────────────────────────────────
MESH_PATH = "/Game/Characters/Huikong/Mesh/SKM_Huikong"
SKELETON_PATH = "/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton"
IDLE_PATH = "/Game/Characters/Huikong/Animations/AM_Huikong_Idle_01"
WALK_PATH = "/Game/Characters/Huikong/Animations/AM_Huikong_Walk_01"
RUN_PATH = "/Game/Characters/Huikong/Animations/AM_Huikong_Run_01"
MAP_PATH = "/Game/Maps/TestLevel_AnimTest"

# ─── Load assets ───────────────────────────────────────────────
mesh = unreal.load_asset(MESH_PATH)
idle = unreal.load_asset(IDLE_PATH)
walk = unreal.load_asset(WALK_PATH)
run = unreal.load_asset(RUN_PATH)

log(f"Mesh: {mesh.get_name() if mesh else 'NOT FOUND'}")
log(f"Idle: {idle.get_name() if idle else 'NOT FOUND'}")
log(f"Walk: {walk.get_name() if walk else 'NOT FOUND'}")
log(f"Run: {run.get_name() if run else 'NOT FOUND'}")

if not all([mesh, idle, walk, run]):
    log_error("Missing assets! Cannot create test level.")
    exit(1)

# ─── Create or open test map ──────────────────────────────────
log("Creating test map...")

editor_level = unreal.EditorLevelLibrary

# Check if map already exists
if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    log(f"Map does not exist, creating: {MAP_PATH}")
    # Create a new level
    try:
        new_world = editor_level.new_level(MAP_PATH)
        log(f"New level created: {new_world}")
    except Exception as e:
        log_error(f"Failed to create level: {e}")
        import traceback
        log_error(traceback.format_exc())
        exit(1)
else:
    log(f"Map exists, loading: {MAP_PATH}")
    try:
        editor_level.load_level(MAP_PATH)
    except Exception as e:
        log_error(f"Failed to load level: {e}")
        exit(1)

# ── Get current world ────────────────────────────────────────
world = editor_level.get_editor_world()
if not world:
    log_error("Cannot get editor world!")
    exit(1)
log(f"Editor world: {world.get_name()}")

# ─── Spawn AnimTestActor ─────────────────────────────────────
log("Spawning AnimTestActor...")

# load_class requires (Outer, Name) signature
actor_class = unreal.load_class(None, "/Script/FirstGame.AnimTestActor")
if not actor_class:
    log_error("AnimTestActor class not found! C++ may not have compiled.")
    exit(1)
log(f"Actor class loaded: {actor_class.get_name()}")

# Spawn at origin
actor = editor_level.spawn_actor_from_class(
    actor_class,
    unreal.Vector(0, 0, 0),
    unreal.Rotator(0, 0, 0)
)

if not actor:
    log_error("Failed to spawn AnimTestActor!")
    exit(1)

log(f"Spawned: {actor.get_name()} at {actor.get_actor_location()}")

# ─── Configure actor properties ───────────────────────────────
log("Configuring actor properties...")

# Set SkeletalMesh
try:
    # The SkeletalMesh is a component, we need to set the mesh asset on it
    skeletal_mesh_comp = actor.get_editor_property("SkeletalMesh")
    if skeletal_mesh_comp:
        skeletal_mesh_comp.set_editor_property("SkeletalMesh", mesh)
        skeletal_mesh_comp.set_editor_property("AnimClass", None)  # No ABP
        log(f"  SkeletalMesh set: {mesh.get_name()}")
    else:
        log_error("  SkeletalMesh component is None!")
except Exception as e:
    log_error(f"  Set SkeletalMesh error: {e}")

# Set animations
try:
    actor.set_editor_property("IdleAnim", idle)
    actor.set_editor_property("WalkAnim", walk)
    actor.set_editor_property("RunAnim", run)
    log("  Idle/Walk/Run animations set")
except Exception as e:
    log_error(f"  Set animations error: {e}")

# Set auto-play
try:
    actor.set_editor_property("bAutoPlay", True)
    actor.set_editor_property("SwitchInterval", 3.0)  # Switch every 3 seconds
    actor.set_editor_property("BlendTime", 0.2)
    log("  Auto-play enabled, 3s switch interval, 0.2s blend")
except Exception as e:
    log_error(f"  Set auto-play error: {e}")

# ─── Save ─────────────────────────────────────────────────────
log("Saving...")

try:
    # Save the actor
    actor.modify()
    # Save the level
    editor_level.save_current_level()
    log(f"Level saved: {MAP_PATH}")
except Exception as e:
    log_error(f"Save error: {e}")
    import traceback
    log_error(traceback.format_exc())

log("=== Test Level Setup Complete ===")
log("")
log("Next: Connect via VNC to verify the actor is in the level")
log("  1. Open VNC (port 5902 via SSH tunnel)")
log("  2. Check Content Browser → Maps → TestLevel_AnimTest")
log("  3. Open the map and verify AnimTestActor with animations")
log("  4. Press Play (PIE) to see Idle → Walk → Run cycling")
