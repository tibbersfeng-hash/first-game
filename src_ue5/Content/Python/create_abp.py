import unreal

unreal.log("[ABP] Creating Animation Blueprint...")

# Check if skeleton exists
skeleton_path = "/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton"
if not unreal.EditorAssetLibrary.does_asset_exist(skeleton_path):
    unreal.log_error(f"[ABP] Skeleton not found: {skeleton_path}")
    exit(1)

skeleton = unreal.load_asset(skeleton_path)
unreal.log(f"[ABP] Using skeleton: {skeleton.get_name()}")

# Create Animation Blueprint
abp_path = "/Game/Characters/Huikong/ABP/ABP_Huikong"
abp_dir = "/Game/Characters/Huikong/ABP"

# Create directory
if not unreal.EditorAssetLibrary.does_directory_exist(abp_dir):
    unreal.EditorAssetLibrary.make_directory(abp_dir)
    unreal.log(f"[ABP] Created directory: {abp_dir}")

# Delete if exists
if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
    unreal.EditorAssetLibrary.delete_asset(abp_path)
    unreal.log(f"[ABP] Deleted old ABP")

# Create Animation Blueprint
try:
    abp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="ABP_Huikong",
        package_path=abp_dir,
        asset_class=unreal.AnimBlueprint,
        factory=unreal.AnimBlueprintFactory()
    )
    
    # Set skeleton
    abp.target_skeleton = skeleton
    
    # Save
    unreal.EditorAssetLibrary.save_asset(abp_path)
    
    unreal.log(f"[ABP] SUCCESS! Created: {abp_path}")
    unreal.log(f"[ABP] Skeleton: {abp.target_skeleton.get_name()}")
    
except Exception as e:
    unreal.log_error(f"[ABP] FAILED: {e}")
