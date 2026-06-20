"""
Create minimal materials + AnimBPs for 4 Candy Dungeon monsters.
Triggered via FIRSTGAME_SETUP_MONSTERS=1 in init_unreal.py.

Simplified approach for P0 prototype:
  - Materials: basic colored materials (no complex graph)
  - AnimBPs: empty AnimBPs bound to each monster's skeleton
"""
import unreal

MONSTERS = [
    {
        "key": "CandyZombie",
        "content_path": "/Game/Monsters/CandyZombie",
        "skeleton_path": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie_Skeleton",
        "mesh_path": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
    },
    {
        "key": "Gingerbread",
        "content_path": "/Game/Monsters/Gingerbread",
        "skeleton_path": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread_Skeleton",
        "mesh_path": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
    },
    {
        "key": "ShadowNinja",
        "content_path": "/Game/Monsters/ShadowNinja",
        "skeleton_path": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja_Skeleton",
        "mesh_path": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
    },
    {
        "key": "ArmoredGum",
        "content_path": "/Game/Monsters/ArmoredGum",
        "skeleton_path": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum_Skeleton",
        "mesh_path": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
    },
]


def ensure_dir(path: str):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_basic_material(monster: dict):
    """Create a basic material that just uses the default Lit shading model."""
    key = monster["key"]
    mat_dir = f"{monster['content_path']}/Materials"
    mat_name = f"M_{key}_Basic"
    mat_path = f"{mat_dir}/{mat_name}"
    ensure_dir(mat_dir)

    if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
        unreal.log(f"  Material {mat_name} already exists, skipping")
        return unreal.load_asset(mat_path)

    factory = unreal.MaterialFactoryNew()
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=mat_name,
        package_path=mat_dir,
        asset_class=unreal.Material,
        factory=factory,
    )
    if not material:
        unreal.log_error(f"  ❌ Failed to create material {mat_name}")
        return None

    # Enable skeletal mesh usage
    try:
        material.set_editor_property('bUsedWithSkeletalMesh', True)
    except Exception as e:
        unreal.log(f"  ⚠️  bUsedWithSkeletalMesh: {e}")

    unreal.EditorAssetLibrary.save_asset(mat_path)
    unreal.log(f"  ✅ Created basic material: {mat_name}")
    return material


def create_anim_bp(monster: dict):
    """Create Animation Blueprint for a monster."""
    key = monster["key"]
    abp_dir = f"{monster['content_path']}/ABP"
    abp_name = f"ABP_{key}"
    abp_path = f"{abp_dir}/{abp_name}"
    ensure_dir(abp_dir)

    if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
        unreal.log(f"  AnimBP {abp_name} already exists, skipping")
        return unreal.load_asset(abp_path)

    skeleton = unreal.load_asset(monster["skeleton_path"])
    if skeleton is None:
        unreal.log_error(f"  ❌ Skeleton not found: {monster['skeleton_path']}")
        return None

    # Create AnimBP using factory
    factory = unreal.AnimBlueprintFactory()

    abp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=abp_name,
        package_path=abp_dir,
        asset_class=unreal.AnimBlueprint,
        factory=factory,
    )

    if abp is None:
        unreal.log_error(f"  ❌ Failed to create AnimBP {abp_name}")
        return None

    # Set skeleton on the created AnimBP
    try:
        abp.target_skeleton = skeleton
    except Exception as e:
        unreal.log(f"  ⚠️  target_skeleton: {e}")
        # Try set_editor_property
        try:
            abp.set_editor_property('target_skeleton', skeleton)
        except Exception as e2:
            unreal.log(f"  ⚠️  set_editor_property target_skeleton: {e2}")

    unreal.EditorAssetLibrary.save_asset(abp_path)
    unreal.log(f"  ✅ Created AnimBP: {abp_name}")
    return abp


def main():
    unreal.log("=" * 60)
    unreal.log("[MONSTER SETUP] Starting Material + AnimBP Setup")
    unreal.log("=" * 60)

    total_mats = 0
    total_abp = 0

    for monster in MONSTERS:
        key = monster["key"]
        unreal.log(f"\n--- {key} ---")

        # Step 1: Create basic material
        mat = create_basic_material(monster)
        if mat:
            total_mats += 1

        # Step 2: Create AnimBP
        abp = create_anim_bp(monster)
        if abp:
            total_abp += 1

    unreal.log("\n" + "=" * 60)
    unreal.log(f"[MONSTER SETUP] Complete: {total_mats}/4 materials, {total_abp}/4 AnimBPs")
    unreal.log("=" * 60)

    unreal.SystemLibrary.execute_console_command(None, 'quit')


main()
