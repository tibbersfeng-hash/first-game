import unreal
import os

unreal.log("[INIT] init_unreal.py executing...")

FBX_PATH = "/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/Mesh/SKM_Huikong_Base.fbx"
DEST_PATH = "/Game/Characters/Huikong/Mesh"

if not os.path.exists(FBX_PATH):
    unreal.log_error(f"[INIT] FBX not found: {FBX_PATH}")
else:
    unreal.log(f"[INIT] FBX found: {FBX_PATH}")
    
    # Create directory
    if not unreal.EditorAssetLibrary.does_directory_exist(DEST_PATH):
        unreal.EditorAssetLibrary.make_directory(DEST_PATH)
    
    # Create import task
    task = unreal.AssetImportTask()
    task.filename = FBX_PATH
    task.destination_path = DEST_PATH
    task.destination_name = "SKM_Huikong"
    task.automated = True
    task.replace_existing = True
    task.save = True
    
    # FBX import options - simple settings
    opts = unreal.FbxImportUI()
    opts.import_mesh = True
    opts.import_textures = False
    opts.import_materials = False
    opts.import_animations = False
    
    task.options = opts
    
    unreal.log("[INIT] Starting import...")
    
    # Execute import
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    
    # Check results
    if task.imported_object_paths:
        unreal.log(f"[INIT] SUCCESS! Imported {len(task.imported_object_paths)} assets:")
        for path in task.imported_object_paths:
            unreal.log(f"[INIT]   {path}")
    else:
        unreal.log_error(f"[INIT] FAILED - no objects created")

unreal.log("[INIT] Done")
