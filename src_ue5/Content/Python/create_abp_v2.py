import unreal

unreal.log("[ABP] Creating Animation Blueprint v2...")

# Check skeleton
skeleton_path = "/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton"
if not unreal.EditorAssetLibrary.does_asset_exist(skeleton_path):
    unreal.log_error(f"[ABP] Skeleton not found: {skeleton_path}")
    exit(1)

skeleton = unreal.load_asset(skeleton_path)
unreal.log(f"[ABP] Using skeleton: {skeleton.get_name()}")

# Create directory
abp_dir = "/Game/Characters/Huikong/ABP"
if not unreal.EditorAssetLibrary.does_directory_exist(abp_dir):
    unreal.EditorAssetLibrary.make_directory(abp_dir)

# Delete if exists
abp_path = f"{abp_dir}/ABP_Huikong"
if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
    unreal.EditorAssetLibrary.delete_asset(abp_path)

# Create using factory
factory = unreal.AnimBlueprintFactory()
factory.target_skeleton = skeleton

unreal.log("[ABP] Creating via factory...")
try:
    abp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="ABP_Huikong",
        package_path=abp_dir,
        asset_class=unreal.AnimBlueprint,
        factory=factory
    )
    
    unreal.EditorAssetLibrary.save_asset(abp_path)
    unreal.log(f"[ABP] SUCCESS: {abp_path}")
    
except Exception as e:
    unreal.log_error(f"[ABP] FAILED: {e}")
    import traceback
    unreal.log_error(f"[ABP] Traceback: {traceback.format_exc()}")
