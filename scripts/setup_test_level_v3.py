"""
Setup Clean Test Level - AnimTestActor only (v3)
==================================================
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

log("=== Clean Setup v3 ===")

# Delete old map
if unreal.EditorAssetLibrary.does_asset_exist(MAP):
    unreal.EditorAssetLibrary.delete_asset(MAP)
    unreal.EditorAssetLibrary.save_deleted_assets()
    log("Deleted old map")

# Create new empty level
result = unreal.EditorLevelLibrary.new_level(MAP)
log(f"Level created: {result}")

world = unreal.EditorLevelLibrary.get_editor_world()
log(f"World: {world.get_name()}")
log(f"Actors: {len(unreal.EditorLevelLibrary.get_all_level_actors())}")

# Spawn AnimTestActor
cls = unreal.load_class(None, "/Script/FirstGame.AnimTestActor")
actor = unreal.EditorLevelLibrary.spawn_actor_from_class(cls, unreal.Vector(0,0,0), unreal.Rotator(0,0,0))
log(f"Spawned: {actor.get_name()}")

# Set mesh
mesh_comp = actor.get_editor_property("SkeletalMesh")
mesh_asset = unreal.load_asset(MESH)
if mesh_comp and mesh_asset:
    mesh_comp.set_editor_property("SkeletalMesh", mesh_asset)
    log("SkeletalMesh set (AnimationMode already set in C++ constructor)")

# Set animations
actor.set_editor_property("IdleAnim", unreal.load_asset(IDLE))
actor.set_editor_property("WalkAnim", unreal.load_asset(WALK))
actor.set_editor_property("RunAnim", unreal.load_asset(RUN))
log("Idle/Walk/Run animations set")

# Playback settings
actor.set_editor_property("bAutoPlay", True)
actor.set_editor_property("SwitchInterval", 3.0)
actor.set_editor_property("BlendTime", 0.2)
log("AutoPlay=True, SwitchInterval=3.0, BlendTime=0.2")

# Save
actor.modify()
unreal.EditorLevelLibrary.save_current_level()
log(f"Saved: {MAP}")
log(f"Total actors: {len(unreal.EditorLevelLibrary.get_all_level_actors())}")
log("=== Done ===")
