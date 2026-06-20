"""
Import 4 Candy Dungeon monster SkeletalMeshes + animations into UE5.
Triggered via FIRSTGAME_IMPORT_MONSTERS=1 environment variable in init_unreal.py.

Pipeline:
  1. Import 4 SkeletalMesh FBXs (each creates Skeleton + SkeletalMesh)
  2. Import 24 animation FBXs (6 per monster, retarget to monster Skeleton)
  3. Exit editor
"""
import unreal
import os
import sys

MONSTERS = [
    {
        "key": "CandyZombie",
        "mesh_fbx": "/home/vipuser/first-game/src_ue5/Content/Monsters/CandyZombie/Mesh/SK_CandyZombie.fbx",
        "anim_dir": "/home/vipuser/first-game/src_ue5/Content/Monsters/CandyZombie/Animations",
        "content_path": "/Game/Monsters/CandyZombie",
        "skeleton_name": "SK_CandyZombie_Skeleton",
    },
    {
        "key": "Gingerbread",
        "mesh_fbx": "/home/vipuser/first-game/src_ue5/Content/Monsters/Gingerbread/Mesh/SK_Gingerbread.fbx",
        "anim_dir": "/home/vipuser/first-game/src_ue5/Content/Monsters/Gingerbread/Animations",
        "content_path": "/Game/Monsters/Gingerbread",
        "skeleton_name": "SK_Gingerbread_Skeleton",
    },
    {
        "key": "ShadowNinja",
        "mesh_fbx": "/home/vipuser/first-game/src_ue5/Content/Monsters/ShadowNinja/Mesh/SK_ShadowNinja.fbx",
        "anim_dir": "/home/vipuser/first-game/src_ue5/Content/Monsters/ShadowNinja/Animations",
        "content_path": "/Game/Monsters/ShadowNinja",
        "skeleton_name": "SK_ShadowNinja_Skeleton",
    },
    {
        "key": "ArmoredGum",
        "mesh_fbx": "/home/vipuser/first-game/src_ue5/Content/Monsters/ArmoredGum/Mesh/SK_ArmoredGum.fbx",
        "anim_dir": "/home/vipuser/first-game/src_ue5/Content/Monsters/ArmoredGum/Animations",
        "content_path": "/Game/Monsters/ArmoredGum",
        "skeleton_name": "SK_ArmoredGum_Skeleton",
    },
]


def ensure_directory(path: str):
    """Create UE5 content directory if it doesn't exist."""
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        unreal.log(f"  Created directory: {path}")


def import_skeletal_mesh(fbx_path: str, dest_path: str, skeleton_name: str) -> bool:
    """Import a SkeletalMesh from FBX.

    Returns True on success.
    """
    if not os.path.exists(fbx_path):
        unreal.log_error(f"  FBX not found: {fbx_path}")
        return False

    mesh_name = os.path.splitext(os.path.basename(fbx_path))[0]

    # Ensure destination directories exist
    mesh_dir = f"{dest_path}/Mesh"
    ensure_directory(dest_path)
    ensure_directory(mesh_dir)

    unreal.log(f"  Importing {mesh_name} from {fbx_path}")

    # Configure import task
    task = unreal.AssetImportTask()
    task.filename = fbx_path
    task.destination_path = mesh_dir
    task.destination_name = mesh_name
    task.automated = True
    task.save = True
    task.replace_existing = True

    # FBX import options for skeletal mesh
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_textures = False
    options.import_materials = False
    options.import_as_skeletal = True
    # Use default skeletal mesh import settings
    # UE5.7 Python API has limited access to FbxSkeletalMeshImportData properties

    # Skeleton - let FBX create a new one
    options.skeleton = None

    task.options = options

    try:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        imported = task.imported_object_paths
        if imported:
            unreal.log(f"  ✅ Imported: {imported}")
            return True
        else:
            unreal.log_error(f"  ❌ No objects imported")
            return False
    except Exception as e:
        unreal.log_error(f"  ❌ Import failed: {e}")
        return False


def import_animations(anim_dir: str, dest_path: str, skeleton) -> int:
    """Import animation FBXs from a directory.

    Returns number of successfully imported animations.
    """
    if not os.path.isdir(anim_dir):
        unreal.log_error(f"  Animation directory not found: {anim_dir}")
        return 0

    anim_dest = f"{dest_path}/Animations"
    ensure_directory(anim_dest)

    fbx_files = sorted([f for f in os.listdir(anim_dir) if f.endswith('.fbx')])
    unreal.log(f"  Found {len(fbx_files)} animation FBXs")

    imported_count = 0
    for fbx_name in fbx_files:
        fbx_path = os.path.join(anim_dir, fbx_name)
        anim_name = os.path.splitext(fbx_name)[0]

        task = unreal.AssetImportTask()
        task.filename = fbx_path
        task.destination_path = anim_dest
        task.destination_name = anim_name
        task.automated = True
        task.save = True
        task.replace_existing = True

        options = unreal.FbxImportUI()
        options.import_mesh = False
        options.import_animations = True
        options.import_textures = False
        options.import_materials = False
        options.skeleton = skeleton
        options.override_full_name = True

        task.options = options

        try:
            unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
            if task.imported_object_paths:
                unreal.log(f"    ✅ {fbx_name} -> {task.imported_object_paths}")
                imported_count += 1
            else:
                unreal.log_error(f"    ❌ {fbx_name}: no objects imported")
        except Exception as e:
            unreal.log_error(f"    ❌ {fbx_name}: {e}")

    return imported_count


def main():
    unreal.log("=" * 60)
    unreal.log("[MONSTER IMPORT] Starting Candy Dungeon Monster Import")
    unreal.log("=" * 60)

    total_meshes = 0
    total_anims = 0

    for monster in MONSTERS:
        key = monster["key"]
        unreal.log(f"\n--- {key} ---")

        # Step 1: Import SkeletalMesh
        success = import_skeletal_mesh(
            monster["mesh_fbx"],
            monster["content_path"],
            monster["skeleton_name"],
        )

        if not success:
            unreal.log_error(f"  Skipping {key} (mesh import failed)")
            continue

        total_meshes += 1

        # Load the newly created skeleton
        skeleton_path = f"{monster['content_path']}/Mesh/{monster['key']}_Skeleton"
        # Try multiple naming conventions
        skeleton = None
        for try_name in [
            f"{monster['content_path']}/Mesh/{monster['key']}_Skeleton",
            f"{monster['content_path']}/Mesh/SK_{key}_Skeleton",
            f"{monster['content_path']}/Mesh/{key}_Skeleton",
        ]:
            if unreal.EditorAssetLibrary.does_asset_exist(try_name):
                skeleton = unreal.load_asset(try_name)
                unreal.log(f"  Found skeleton: {try_name}")
                break

        if skeleton is None:
            # List what was actually created
            mesh_assets = unreal.EditorAssetLibrary.list_assets(
                f"{monster['content_path']}/Mesh", recursive=False
            )
            unreal.log(f"  Mesh assets created: {mesh_assets}")
            # Try to find the skeleton from the mesh
            for asset_path in mesh_assets:
                if "Skeleton" in asset_path:
                    skeleton = unreal.load_asset(asset_path)
                    unreal.log(f"  Using skeleton: {asset_path}")
                    break

        if skeleton is None:
            unreal.log_error(f"  ❌ Could not find skeleton for {key}")
            continue

        # Step 2: Import animations
        anim_count = import_animations(
            monster["anim_dir"],
            monster["content_path"],
            skeleton,
        )
        total_anims += anim_count
        unreal.log(f"  Animations: {anim_count} imported")

    # Summary
    unreal.log("\n" + "=" * 60)
    unreal.log(f"[MONSTER IMPORT] Complete: {total_meshes}/4 meshes, {total_anims}/24 anims")
    unreal.log("=" * 60)

    # Quit editor
    unreal.SystemLibrary.execute_console_command(None, 'quit')


main()
