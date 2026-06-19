import unreal
import os

unreal.log("[INIT] init_unreal.py executing...")

# Create Animation Blueprint
try:
    skeleton = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton")
    if skeleton:
        unreal.log(f"[INIT] Skeleton loaded: {skeleton.get_name()}")
        
        # Create directory
        abp_dir = "/Game/Characters/Huikong/ABP"
        if not unreal.EditorAssetLibrary.does_directory_exist(abp_dir):
            unreal.EditorAssetLibrary.make_directory(abp_dir)
        
        # Create ABP
        abp_path = f"{abp_dir}/ABP_Huikong"
        if not unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            factory = unreal.AnimBlueprintFactory()
            abp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                asset_name="ABP_Huikong",
                package_path=abp_dir,
                asset_class=unreal.AnimBlueprint,
                factory=factory
            )
            
            if abp:
                try:
                    abp.set_editor_property("TargetSkeleton", skeleton)
                except:
                    pass
                unreal.EditorAssetLibrary.save_asset(abp_path)
                unreal.log(f"[INIT] ABP created: {abp_path}")
            else:
                unreal.log_error("[INIT] Failed to create ABP")
        else:
            unreal.log(f"[INIT] ABP already exists: {abp_path}")
    else:
        unreal.log("[INIT] Skeleton not found yet")
except Exception as e:
    unreal.log_error(f"[INIT] Error: {e}")

unreal.log("[INIT] Done")
