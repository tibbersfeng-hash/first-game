import unreal
import os
import glob

unreal.log("[V3] Importing animations...")

# Load skeleton
skeleton = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton")
unreal.log(f"[V3] Skeleton: {skeleton.get_name()}")

# Import all original animations
anim_files = [
    ("待机-1.fbx", "AM_Huikong_Idle_01"),
    ("待机-2.fbx", "AM_Huikong_Idle_02"),
    ("走路-1.fbx", "AM_Huikong_Walk_01"),
    ("奔跑.fbx", "AM_Huikong_Run_01"),
    ("回旋踢.fbx", "AM_Huikong_LightAttack_01"),
    ("受击.fbx", "AM_Huikong_HitReaction"),
    ("落地.fbx", "AM_Huikong_Landing"),
]

dest_path = "/Game/Characters/Huikong/Animations"
success = 0

for fbx_name, anim_name in anim_files:
    fbx_path = f"/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/_old_anim/{fbx_name}"
    
    if not os.path.exists(fbx_path):
        unreal.log_error(f"[V3] File not found: {fbx_path}")
        continue
    
    # Delete old asset if exists
    old_asset = f"{dest_path}/{anim_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(old_asset):
        unreal.EditorAssetLibrary.delete_asset(old_asset)
    
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
    
    unreal.log(f"[V3] Importing: {anim_name}")
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    
    if task.imported_object_paths:
        unreal.log(f"[V3]   ✓ {anim_name}")
        for path in task.imported_object_paths:
            asset = unreal.EditorAssetLibrary.load_asset(path)
            unreal.log(f"[V3]     Type: {asset.get_class().get_name()}")
        success += 1
    else:
        unreal.log_error(f"[V3]   ✗ {anim_name}")

unreal.log(f"[V3] Done: {success}/{len(anim_files)} animations imported")
