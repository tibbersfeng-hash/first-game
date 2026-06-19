import unreal

unreal.log("[ABP] Creating Animation Blueprint (simple method)...")

# Load skeleton
skeleton = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton")
if not skeleton:
    unreal.log_error("[ABP] Failed to load skeleton")
    exit(1)

unreal.log(f"[ABP] Skeleton loaded: {skeleton.get_name()}")

# Create ABP directory
abp_dir = "/Game/Characters/Huikong/ABP"
if not unreal.EditorAssetLibrary.does_directory_exist(abp_dir):
    unreal.EditorAssetLibrary.make_directory(abp_dir)
    unreal.log(f"[ABP] Created directory: {abp_dir}")

# Delete if exists
abp_path = f"{abp_dir}/ABP_Huikong"
if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
    unreal.EditorAssetLibrary.delete_asset(abp_path)
    unreal.log("[ABP] Deleted old ABP")

# Create ABP using AnimBlueprintFactory
try:
    factory = unreal.AnimBlueprintFactory()
    
    # Create the asset
    abp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="ABP_Huikong",
        package_path=abp_dir,
        asset_class=unreal.AnimBlueprint,
        factory=factory
    )
    
    if abp:
        # Try to set skeleton via different methods
        try:
            abp.set_editor_property("TargetSkeleton", skeleton)
            unreal.log("[ABP] Set skeleton via set_editor_property")
        except:
            try:
                abp.TargetSkeleton = skeleton
                unreal.log("[ABP] Set skeleton via direct assignment")
            except Exception as e:
                unreal.log_warning(f"[ABP] Could not set skeleton: {e}")
        
        # Save
        unreal.EditorAssetLibrary.save_asset(abp_path)
        unreal.log(f"[ABP] SUCCESS: Created {abp_path}")
        unreal.log(f"[ABP] ABP class: {abp.get_class().get_name()}")
    else:
        unreal.log_error("[ABP] FAILED: create_asset returned None")
        
except Exception as e:
    unreal.log_error(f"[ABP] FAILED: {e}")
    import traceback
    unreal.log_error(f"[ABP] Traceback: {traceback.format_exc()}")
