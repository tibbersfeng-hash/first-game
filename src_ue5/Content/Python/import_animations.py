import unreal
import os
import glob

unreal.log("[ANIM] Importing animations...")

# Load the skeleton
skeleton_path = "/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton"
if not unreal.EditorAssetLibrary.does_asset_exist(skeleton_path):
    unreal.log_error(f"[ANIM] Skeleton not found: {skeleton_path}")
    exit(1)

skeleton = unreal.EditorAssetLibrary.load_asset(skeleton_path)
unreal.log(f"[ANIM] Loaded skeleton: {skeleton.get_name()}")

# Import animations
anim_dir = "/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/Animations"
anim_files = glob.glob(os.path.join(anim_dir, "AM_Huikong_*.fbx"))
unreal.log(f"[ANIM] Found {len(anim_files)} animation files")

dest_path = "/Game/Characters/Huikong/Animations"
if not unreal.EditorAssetLibrary.does_directory_exist(dest_path):
    unreal.EditorAssetLibrary.make_directory(dest_path)

success = 0
for fbx_path in anim_files:
    anim_name = os.path.basename(fbx_path).replace(".fbx", "")
    unreal.log(f"[ANIM] Importing: {anim_name}")
    
    task = unreal.AssetImportTask()
    task.filename = fbx_path
    task.destination_path = dest_path
    task.destination_name = anim_name
    task.automated = True
    task.replace_existing = True
    task.save = True
    
    opts = unreal.FbxImportUI()
    opts.import_mesh = False
    opts.import_animations = True
    opts.import_textures = False
    opts.import_materials = False
    opts.skeleton = skeleton
    opts.override_full_name = True
    
    task.options = opts
    
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    
    if task.imported_object_paths:
        unreal.log(f"[ANIM]   ✓ {anim_name}")
        success += 1
    else:
        unreal.log_error(f"[ANIM]   ✗ {anim_name}")

unreal.log(f"[ANIM] Done: {success}/{len(anim_files)} animations imported")
