import unreal
import os

unreal.log("[V4] Forcing animation import...")

# Load skeleton
skeleton = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton")

fbx_path = "/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/_old_anim/待机-1.fbx"
dest_path = "/Game/Characters/Huikong/Animations"

# Delete old
old_asset = f"{dest_path}/AM_Huikong_Idle_01"
if unreal.EditorAssetLibrary.does_asset_exist(old_asset):
    unreal.EditorAssetLibrary.delete_asset(old_asset)

task = unreal.AssetImportTask()
task.filename = fbx_path
task.destination_path = dest_path
task.destination_name = "AM_Huikong_Idle_01"
task.automated = True
task.replace_existing = True
task.save = True

opts = unreal.FbxImportUI()

# Try different settings
opts.import_mesh = False
opts.import_as_skeletal = True  # Force skeletal
opts.import_animations = True
opts.import_textures = False
opts.import_materials = False
opts.skeleton = skeleton
opts.override_full_name = True

# Try setting mesh type to animation
try:
    opts.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    unreal.log("[V4] Set mesh_type_to_import = FBXIT_ANIMATION")
except:
    unreal.log("[V4] Could not set mesh_type_to_import")

task.options = opts

unreal.log("[V4] Importing...")
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

if task.imported_object_paths:
    for path in task.imported_object_paths:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        unreal.log(f"[V4] Created: {path}")
        unreal.log(f"[V4] Type: {asset.get_class().get_name()}")
else:
    unreal.log_error(f"[V4] FAILED")
