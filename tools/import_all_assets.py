"""
UE5 Editor Python Script - Auto import Huikong 3D assets (UE5.7)
Mesh -> Skeleton -> 7 Animations
"""
import unreal
import os
import sys

FBX_SRC = os.path.join(unreal.Paths.project_dir(), "Content", "_fbx_staging")
HUIKONG_BASE = "/Game/Characters/Huikong"
MESH_NAME = "SKM_Huikong"
SKELETON_NAME = "SK_Huikong"

def log(msg):
    unreal.log("[Huikong] " + str(msg))
    print("[Huikong] " + str(msg))

def refresh(path="/Game/Characters/"):
    try:
        unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([path], recursive=True)
    except:
        pass

def ensure_dirs():
    for d in ["Mesh", "Skeleton", "Physics", "Animations", "ABP", "Materials"]:
        path = HUIKONG_BASE + "/" + d
        if not unreal.EditorAssetLibrary.does_directory_exist(path):
            unreal.EditorAssetLibrary.make_directory(path)
            log("Created dir: " + path)

def import_mesh():
    """Import main mesh, return Skeleton reference"""
    dest = HUIKONG_BASE + "/Mesh/" + MESH_NAME
    fbx_path = os.path.join(FBX_SRC, "model_3.fbx")

    if unreal.EditorAssetLibrary.does_asset_exist(dest):
        log("Mesh already exists: " + dest)
        skel_path = HUIKONG_BASE + "/Skeleton/" + SKELETON_NAME
        if unreal.EditorAssetLibrary.does_asset_exist(skel_path):
            return unreal.load_asset(skel_path)
        return None

    if not os.path.exists(fbx_path):
        log("Error: " + fbx_path + " does not exist")
        return None

    log("Importing mesh: " + fbx_path)

    task = unreal.AssetImportTask()
    task.filename = fbx_path
    task.destination_path = HUIKONG_BASE + "/Mesh"
    task.destination_name = MESH_NAME
    task.automated = True
    task.replace_existing = True
    task.save = True

    opts = unreal.FbxImportUI()
    opts.import_mesh = True
    opts.import_animations = False
    opts.import_textures = False
    opts.import_materials = True
    opts.create_physics_asset = True
    opts.automated_import_scale = 1.0
    task.options = opts

    try:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        refresh()

        if unreal.EditorAssetLibrary.does_asset_exist(dest):
            log("Mesh imported: " + dest)
            skel_src = HUIKONG_BASE + "/Mesh/" + SKELETON_NAME
            skel_dst = HUIKONG_BASE + "/Skeleton/" + SKELETON_NAME
            if unreal.EditorAssetLibrary.does_asset_exist(skel_src):
                unreal.EditorAssetLibrary.rename_asset(skel_src, skel_dst)
                log("Skeleton moved to: " + skel_dst)
                refresh()
                return unreal.load_asset(skel_dst)
            else:
                log("Auto-generated skeleton not found at: " + skel_src)
                log("Please open SKM_Huikong -> Skeleton -> Save As")
                return None
        else:
            log("Mesh asset not found after import: " + dest)
            return None
    except Exception as e:
        log("Mesh import failed: " + str(e))
        return None

def import_one_animation(en_name, skeleton):
    fbx = os.path.join(FBX_SRC, en_name + ".fbx")

    if not os.path.exists(fbx):
        log("  Skip " + en_name + ": file not found")
        return False

    dest = HUIKONG_BASE + "/Animations/" + en_name
    if unreal.EditorAssetLibrary.does_asset_exist(dest):
        log("  Already exists: " + en_name)
        return True

    log("  Importing animation: " + en_name)

    task = unreal.AssetImportTask()
    task.filename = fbx
    task.destination_path = HUIKONG_BASE + "/Animations"
    task.destination_name = en_name
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

    try:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        refresh()
        if unreal.EditorAssetLibrary.does_asset_exist(dest):
            log("  OK " + en_name)
            return True
        else:
            log("  FAIL " + en_name + ": asset not found after import")
            return False
    except Exception as e:
        log("  FAIL " + en_name + ": " + str(e))
        return False

def main():
    log("=" * 60)
    log("Huikong 3D Asset Auto Import (UE5.7)")
    log("FBX source: " + FBX_SRC)
    log("Target: " + HUIKONG_BASE)
    log("=" * 60)

    if not os.path.exists(FBX_SRC):
        log("Error: " + FBX_SRC + " does not exist!")
        sys.exit(1)

    # Step 1
    log("")
    log("[Step 1/4] Creating directories...")
    ensure_dirs()

    # Step 2
    log("")
    log("[Step 2/4] Importing mesh + skeleton...")
    skeleton = import_mesh()
    if not skeleton:
        log("Skeleton not available, trying to load existing...")
        skel_path = HUIKONG_BASE + "/Skeleton/" + SKELETON_NAME
        if unreal.EditorAssetLibrary.does_asset_exist(skel_path):
            skeleton = unreal.load_asset(skel_path)
            log("Loaded existing skeleton: " + skel_path)
        else:
            log("Cannot get skeleton. Please import model_3.fbx manually in UE5 Editor")
            log("Then run: exec(open(r'import_huikong_assets.py').read())")
            sys.exit(1)

    # Step 3
    log("")
    log("[Step 3/4] Importing animations...")
    anims = [
        "Anim_Huikong_Idle_01", "Anim_Huikong_Idle_02",
        "Anim_Huikong_Walk_01", "Anim_Huikong_Run_01",
        "Anim_Huikong_LightAttack_01", "Anim_Huikong_HitReaction",
        "Anim_Huikong_Landing",
    ]
    ok = 0
    for a in anims:
        if import_one_animation(a, skeleton):
            ok += 1

    # Step 4
    log("")
    log("[Step 4/4] Complete!")
    log("  Mesh: 1/1")
    log("  Animations: " + str(ok) + "/" + str(len(anims)))
    log("  Asset path: " + HUIKONG_BASE + "/Animations")
    log("=" * 60)
    log("Now view in Content Browser:")
    log("  " + HUIKONG_BASE + "/Mesh/" + MESH_NAME)
    log("  " + HUIKONG_BASE + "/Skeleton/" + SKELETON_NAME)
    for a in anims:
        log("  " + HUIKONG_BASE + "/Animations/" + a)
    log("=" * 60)

main()
