import unreal
import os
import glob

unreal.log("[BATCH] Importing all original animations...")

# Load skeleton
skeleton = unreal.EditorAssetLibrary.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton")
if not skeleton:
    unreal.log_error("[BATCH] Skeleton not found!")
    exit(1)

# Find all original animation FBX files
anim_dir = "/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/_old_anim"
anim_files = glob.glob(os.path.join(anim_dir, "*.fbx"))
unreal.log(f"[BATCH] Found {len(anim_files)} animation files")

# Create destination directory
dest_path = "/Game/Characters/Huikong/Animations"
if not unreal.EditorAssetLibrary.does_directory_exist(dest_path):
    unreal.EditorAssetLibrary.make_directory(dest_path)

# Map Chinese names to English
name_map = {
    "待机-1": "AM_Huikong_Idle_01",
    "待机-2": "AM_Huikong_Idle_02",
    "走路-1": "AM_Huikong_Walk_01",
    "奔跑": "AM_Huikong_Run_01",
    "回旋踢": "AM_Huikong_LightAttack_01",
    "受击": "AM_Huikong_HitReaction",
    "落地": "AM_Huikong_Landing",
}

success = 0
for fbx_path in anim_files:
    base_name = os.path.basename(fbx_path).replace(".fbx", "")
    anim_name = name_map.get(base_name, f"AM_Huikong_{base_name}")
    
    unreal.log(f"[BATCH] Importing: {base_name} -> {anim_name}")
    
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
    
    task.options = opts
    
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    
    if task.imported_object_paths:
        unreal.log(f"[BATCH]   ✓ {anim_name}")
        success += 1
    else:
        unreal.log_error(f"[BATCH]   ✗ {anim_name}")

unreal.log(f"[BATCH] Done: {success}/{len(anim_files)} animations imported")
