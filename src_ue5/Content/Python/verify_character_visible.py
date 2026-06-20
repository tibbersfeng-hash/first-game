"""
verify_character_visible.py
===========================
Runs inside Game/PIE mode. Ensures the Huikong character is visible by:
  1. Finding AnimTestActor in the world
  2. Teleporting the player pawn in front of it (facing the actor)
  3. Spawning a DirectionalLight + SkyLight for illumination
  4. Waiting for render/lighting to settle
  5. Taking a HighResShot screenshot
"""
import unreal
import time

LOG = "[VERIFY-VISIBLE]"

def log(msg):
    unreal.log(f"{LOG} {msg}")

def log_error(msg):
    unreal.log_error(f"{LOG} {msg}")

log("Starting visibility verification...")

# ─── Wait for world to be ready ────────────────────────────────
time.sleep(8)

world = unreal.SystemLibrary.get_editor_world() if hasattr(unreal.SystemLibrary, 'get_editor_world') else None
if not world:
    # In standalone game mode
    worlds = unreal.Object.find_objects_include_outer("World")
    world = worlds[0] if worlds else None
    log(f"Game world (fallback): {world}")
else:
    log(f"Editor world: {world}")

if not world:
    log_error("No world found!")
    unreal.SystemLibrary.execute_console_command(None, "quit")

# ─── Find AnimTestActor ────────────────────────────────────────
actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.load_class(None, "/Script/FirstGame.AnimTestActor"))
if not actors:
    log_error("AnimTestActor not found in level!")
    unreal.SystemLibrary.execute_console_command(None, "quit")

anim_actor = actors[0]
actor_loc = anim_actor.get_actor_location()
log(f"AnimTestActor at {actor_loc}")

# ─── Teleport player in front of actor ─────────────────────────
player_controller = unreal.GameplayStatics.get_player_controller(world, 0)
if player_controller:
    pawn = player_controller.k2_get_pawn()
    if pawn:
        # Stand 250 units in front of actor (in +X direction), facing toward actor (-X)
        cam_pos = actor_loc + unreal.Vector(250, 0, 20)
        cam_rot = unreal.Rotator(0, 180, 0)
        pawn.set_actor_location_and_rotation(cam_pos, cam_rot, False, None, False)
        player_controller.set_control_rotation(cam_rot)
        log(f"Player teleported to {cam_pos}, facing actor")
    else:
        log_error("No player pawn found!")
else:
    log_error("No player controller found!")

# ─── Spawn lights ──────────────────────────────────────────────
# DirectionalLight
light_class = unreal.load_class(None, "/Script/Engine.DirectionalLight")
sun = unreal.GameplayStatics.begin_spawning_actor_from_class(world, light_class, actor_loc + unreal.Vector(0, 0, 500), unreal.Rotator(-45, 0, 0))
if sun:
    light_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    if light_comp:
        light_comp.set_editor_property("Intensity", 8.0)
    sun.finish_spawning_actor(actor_loc + unreal.Vector(0, 0, 500), unreal.Rotator(-45, 0, 0))
    log(f"DirectionalLight spawned: {sun.get_name()}")

# SkyLight
sky_class = unreal.load_class(None, "/Script/Engine.SkyLight")
sky = unreal.GameplayStatics.begin_spawning_actor_from_class(world, sky_class, actor_loc + unreal.Vector(0, 0, 300), unreal.Rotator(0, 0, 0))
if sky:
    sky.finish_spawning_actor(actor_loc + unreal.Vector(0, 0, 300), unreal.Rotator(0, 0, 0))
    log(f"SkyLight spawned: {sky.get_name()}")

# ─── Wait for lighting/frame to settle ─────────────────────────
log("Waiting 5s for lighting to settle...")
time.sleep(5)

# ─── Take screenshot ───────────────────────────────────────────
log("Taking screenshot...")
try:
    unreal.SystemLibrary.execute_console_command(world, "HighResShot 1920x1080")
    log("HighResShot executed")
except Exception as e:
    log_error(f"HighResShot failed: {e}")
    try:
        unreal.SystemLibrary.execute_console_command(world, "shot")
        log("Fallback 'shot' executed")
    except Exception as e2:
        log_error(f"shot also failed: {e2}")

# ─── Hold briefly so screenshot is captured before quit ────────
time.sleep(3)
log("Verification complete")
unreal.SystemLibrary.execute_console_command(world, "quit")
