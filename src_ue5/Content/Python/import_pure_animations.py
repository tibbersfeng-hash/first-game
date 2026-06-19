import unreal
import os
import glob

unreal.log("[PureAnim] Importing pure animation FBX files...")

# Load skeleton
skeleton_path = "/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton"
if not unreal.EditorAssetLibrary.does_asset_exist(skeleton_path):
    unreal.log_error(f"[PureAnim] Skeleton not found: {skeleton_path}")
    exit(1)

skeleton = unreal.load_asset(skeleton_path)
unreal.log(f"[PureAnim] Loaded skeleton: {skeleton.get_name()}")

# Delete old StaticMesh animations
old_anims = [
    "/Game/Characters/Huikong/Animations/AM_Huikong_Idle_01",
    "/Game/Characters/Huikong/Animations/AM_Huikong_Idle_02",
    "/Game/Characters/Huikong/Animations/AM_Huikong_Walk_01",
    "/Game/Characters/Huikong/Animations/AM_Huikong_Run_01",
    "/Game/Characters/Huikong/Animations/AM_Huikong_LightAttack_01",
    "/Game/Characters/Huikong/Animations/AM_Huikong_HitReaction",
    "/Game/Characters/Huikong/Animations/AM_Huikong_Landing",
]

for anim_path in old_anims:
    if unreal.EditorAssetLibrary.does_asset_exist(anim_path):
        unreal.EditorAssetLibrary.delete_asset(anim_path)
        unreal.log(f"[PureAnim] Deleted: {anim_path}")

# Import pure animations
anim_dir = "/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/Animations"
pure_anims = glob.glob(os.path.join(anim_dir, "pure_AM_Huikong_*.fbx"))
unreal.log(f"[PureAnim] Found {len(pure_anims)} pure animation files")

dest_path = "/Game/Characters/Huikong/Animations"
success = 0

for fbx_path in pure_anims:
    anim_name = os.path.basename(fbx_path).replace("pure_", "").replace(".fbx", "")
    unreal.log(f"[PureAnim] Importing: {anim_name}")
    
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
        unreal.log(f"[PureAnim]   ✓ {anim_name}")
        success += 1
    else:
        unreal.log_error(f"[PureAnim]   ✗ {anim_name}")

unreal.log(f"[PureAnim] Done: {success}/{len(pure_anims)} animations imported")
