import unreal
import os

unreal.log("[INIT] init_unreal.py executing...")

FBX_PATH = "/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/Mesh/model_3.fbx"
DEST_PATH = "/Game/Characters/Huikong/Mesh"

if not os.path.exists(FBX_PATH):
    unreal.log_error(f"[INIT] FBX not found: {FBX_PATH}")
else:
    unreal.log(f"[INIT] FBX found: {FBX_PATH}")
    if not unreal.EditorAssetLibrary.does_directory_exist(DEST_PATH):
        unreal.EditorAssetLibrary.make_directory(DEST_PATH)
    
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
    
    task.options = opts
    
    unreal.log("[INIT] Starting import...")
    try:
        result = unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        unreal.log(f"[INIT] Import completed, tasks: {len(result)}")
        for t in result:
            for p in t.imported_object_paths:
                unreal.log(f"[INIT] Imported: {p}")
    except Exception as e:
        unreal.log_error(f"[INIT] Import failed: {e}")

unreal.log("[INIT] Done")
unreal.SystemLibrary.quit_editor()
