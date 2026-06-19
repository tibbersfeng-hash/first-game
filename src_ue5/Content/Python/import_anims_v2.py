import unreal
import os

unreal.log("[V2] Importing animations from original FBX...")

# Load skeleton
skeleton = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton")
unreal.log(f"[V2] Skeleton: {skeleton.get_name()}")

# Import one test animation
fbx_path = "/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/_old_anim/待机-1.fbx"
dest_path = "/Game/Characters/Huikong/Animations"

# Delete old asset if exists
old_asset = f"{dest_path}/Test_Idle"
if unreal.EditorAssetLibrary.does_asset_exist(old_asset):
    unreal.EditorAssetLibrary.delete_asset(old_asset)

task = unreal.AssetImportTask()
task.filename = fbx_path
task.destination_path = dest_path
task.destination_name = "Test_Idle"
task.automated = True
task.replace_existing = True
task.save = True

opts = unreal.FbxImportUI()
opts.import_mesh = False
opts.import_as_skeletal = True  # Force skeletal import
opts.import_animations = True
opts.import_textures = False
opts.import_materials = False
opts.skeleton = skeleton
opts.override_full_name = True

# Animation settings
opts.animation_import_data.import_custom_attribute = False
opts.animation_import_data.import_animation = True

task.options = opts

unreal.log("[V2] Starting import...")
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

if task.imported_object_paths:
    unreal.log(f"[V2] SUCCESS!")
    for path in task.imported_object_paths:
        unreal.log(f"[V2]   {path}")
        # Check asset type
        asset = unreal.EditorAssetLibrary.load_asset(path)
        unreal.log(f"[V2]   Type: {asset.get_class().get_name()}")
else:
    unreal.log_error(f"[V2] FAILED - no objects created")
    unreal.log_error(f"[V2] Task result: {task.result}")
