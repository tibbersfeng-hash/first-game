import unreal
import os

unreal.log("[FIX] Importing SkeletalMesh...")

FBX_PATH = "/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/Mesh/SKM_Huikong_Base.fbx"
DEST_PATH = "/Game/Characters/Huikong/Mesh"

# Delete old asset if exists
old_asset = "/Game/Characters/Huikong/Mesh/SKM_Huikong"
if unreal.EditorAssetLibrary.does_asset_exist(old_asset):
    unreal.EditorAssetLibrary.delete_asset(old_asset)
    unreal.log(f"[FIX] Deleted old asset: {old_asset}")

task = unreal.AssetImportTask()
task.filename = FBX_PATH
task.destination_path = DEST_PATH
task.destination_name = "SKM_Huikong"
task.automated = True
task.replace_existing = True
task.save = True

opts = unreal.FbxImportUI()
opts.import_mesh = True
opts.import_textures = False
opts.import_materials = False
opts.import_animations = False

# Force skeletal mesh import
opts.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH

task.options = opts

unreal.log("[FIX] Starting import...")
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

if task.imported_object_paths:
    unreal.log(f"[FIX] SUCCESS! Created:")
    for path in task.imported_object_paths:
        unreal.log(f"[FIX]   {path}")
        # Check asset type
        asset = unreal.EditorAssetLibrary.load_asset(path)
        unreal.log(f"[FIX]   Type: {asset.get_class().get_name()}")
else:
    unreal.log_error(f"[FIX] FAILED")

unreal.log("[FIX] Done")
