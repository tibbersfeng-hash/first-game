"""
Fix HeadlessPIETest map - create proper copy with correct internal name
========================================================================
"""
import unreal

LOG = "[FIX]"
def log(msg):
    unreal.log(f"{LOG} {msg}")

# Delete the broken copy
bad_path = "/Game/Maps/HeadlessPIETest"
if unreal.EditorAssetLibrary.does_asset_exist(bad_path):
    unreal.EditorAssetLibrary.delete_asset(bad_path)
    unreal.EditorAssetLibrary.save_deleted_assets()
    log("Deleted broken HeadlessPIETest copy")

# Create a NEW empty level named HeadlessPIETest
result = unreal.EditorLevelLibrary.new_level(bad_path)
log(f"Created new HeadlessPIETest: {result}")

world = unreal.EditorLevelLibrary.get_editor_world()
log(f"World: {world.get_name()}")

# Spawn AnimTestActor in this map too
cls = unreal.load_class(None, "/Script/FirstGame.AnimTestActor")
actor = unreal.EditorLevelLibrary.spawn_actor_from_class(cls, unreal.Vector(0,0,0), unreal.Rotator(0,0,0))
log(f"Spawned: {actor.get_name()}")

# Configure
mesh_comp = actor.get_editor_property("SkeletalMesh")
mesh_asset = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong")
if mesh_comp and mesh_asset:
    mesh_comp.set_editor_property("SkeletalMesh", mesh_asset)
    log("SkeletalMesh set")

actor.set_editor_property("IdleAnim", unreal.load_asset("/Game/Characters/Huikong/Animations/AM_Huikong_Idle_01"))
actor.set_editor_property("WalkAnim", unreal.load_asset("/Game/Characters/Huikong/Animations/AM_Huikong_Walk_01"))
actor.set_editor_property("RunAnim", unreal.load_asset("/Game/Characters/Huikong/Animations/AM_Huikong_Run_01"))
actor.set_editor_property("bAutoPlay", True)
actor.set_editor_property("SwitchInterval", 3.0)
actor.set_editor_property("BlendTime", 0.2)
log("Animations and settings configured")

actor.modify()
unreal.EditorLevelLibrary.save_current_level()
log(f"Saved HeadlessPIETest. Actors: {len(unreal.EditorLevelLibrary.get_all_level_actors())}")

# Now go back to our TestLevel_AnimTest
log("Loading TestLevel_AnimTest...")
unreal.EditorLevelLibrary.load_level("/Game/Maps/TestLevel_AnimTest")
world2 = unreal.EditorLevelLibrary.get_editor_world()
log(f"Now in: {world2.get_name()}")

log("=== Fix complete ===")
log("Both maps now have AnimTestActor with correct internal names")
log("PIE should now load HeadlessPIETest (which has our actor)")
