import unreal
import os
import sys

unreal.log("[Import] Starting mesh import...")

PROJECT_DIR = unreal.Paths.project_content_dir()
FBX_PATH = "/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/Mesh/model_3.fbx"
DEST_PATH = "/Game/Characters/Huikong/Mesh"

# Create directory if needed
if not unreal.EditorAssetLibrary.does_directory_exist(DEST_PATH):
    unreal.EditorAssetLibrary.make_directory(DEST_PATH)
    unreal.log(f"[Import] Created directory: {DEST_PATH}")

# Check FBX exists
if not os.path.exists(FBX_PATH):
    unreal.log_error(f"[Import] FBX not found: {FBX_PATH}")
    sys.exit(1)

unreal.log(f"[Import] FBX: {FBX_PATH}")
unreal.log(f"[Import] Dest: {DEST_PATH}")

# Create import task
task = unreal.AssetImportTask()
task.filename = FBX_PATH
task.destination_path = DEST_PATH
task.destination_name = "SKM_Huikong"
task.automated = True
task.replace_existing = True
task.save = True

# FBX import options
opts = unreal.FbxImportUI()
opts.import_mesh = True
opts.import_textures = False
opts.import_materials = False
opts.import_animations = False
opts.skeleton = unreal.Object()  # Will create new
opts.override_full_name = True

# Mesh import options
mesh_opts = opts.skeletal_mesh_import_data
mesh_opts.import_mesh_body_setup = unreal.FBodySetup()
mesh_opts.import_translation = unreal.Vector(0, 0, 0)
mesh_opts.import_rotation = unreal.Rotator(0, 0, 0)
mesh_opts.import_uniform_scale = 1.0

task.options = opts

unreal.log("[Import] Executing import...")
try:
    result = unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    unreal.log(f"[Import] Import tasks completed: {len(result)}")
    
    # Check result
    dest_asset = f"{DEST_PATH}/SKM_Huikong"
    if unreal.EditorAssetLibrary.does_asset_exist(dest_asset):
        unreal.log(f"[Import] SUCCESS: {dest_asset}")
    else:
        unreal.log(f"[Import] Asset not found at expected path, checking imports...")
        for imp_task in result:
            for path in imp_task.imported_object_paths:
                unreal.log(f"[Import] Imported: {path}")
except Exception as e:
    unreal.log_error(f"[Import] FAILED: {e}")

unreal.log("[Import] Done.")
