"""
Create 4 Enemy Blueprint classes (BP_CandyZombie, etc.) extending ABaseEnemy.
Triggered via FIRSTGAME_CREATE_ENEMY_BPS=1 in init_unreal.py.
"""
import unreal

MONSTERS = [
    {
        "key": "CandyZombie",
        "enemy_type": 0,  # EEnemyType::CandyZombie
        "content_path": "/Game/Monsters/CandyZombie",
        "mesh_path": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
        "abp_path": "/Game/Monsters/CandyZombie/ABP/ABP_CandyZombie",
        "capsule_radius": 30.0,
        "capsule_half_height": 50.0,
    },
    {
        "key": "Gingerbread",
        "enemy_type": 1,  # EEnemyType::Gingerbread
        "content_path": "/Game/Monsters/Gingerbread",
        "mesh_path": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
        "abp_path": "/Game/Monsters/Gingerbread/ABP/ABP_Gingerbread",
        "capsule_radius": 28.0,
        "capsule_half_height": 45.0,
    },
    {
        "key": "ShadowNinja",
        "enemy_type": 2,  # EEnemyType::ShadowNinja
        "content_path": "/Game/Monsters/ShadowNinja",
        "mesh_path": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
        "abp_path": "/Game/Monsters/ShadowNinja/ABP/ABP_ShadowNinja",
        "capsule_radius": 25.0,
        "capsule_half_height": 55.0,
    },
    {
        "key": "ArmoredGum",
        "enemy_type": 3,  # EEnemyType::ArmoredGum
        "content_path": "/Game/Monsters/ArmoredGum",
        "mesh_path": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
        "abp_path": "/Game/Monsters/ArmoredGum/ABP/ABP_ArmoredGum",
        "capsule_radius": 35.0,
        "capsule_half_height": 55.0,
    },
]


def create_enemy_blueprint(monster: dict) -> bool:
    """Create a Blueprint class extending ABaseEnemy for a specific monster."""
    key = monster["key"]
    bp_dir = f"{monster['content_path']}/Blueprints"
    bp_name = f"BP_{key}"
    bp_path = f"{bp_dir}/{bp_name}"

    if not unreal.EditorAssetLibrary.does_directory_exist(bp_dir):
        unreal.EditorAssetLibrary.make_directory(bp_dir)

    if unreal.EditorAssetLibrary.does_asset_exist(bp_path):
        unreal.log(f"  BP_{key} already exists, skipping")
        return True

    # Load parent class using the correct UE5 Python API
    parent_class = unreal.load_class(None, "/Script/FirstGame.BaseEnemy")
    if parent_class is None:
        unreal.log_error(f"  ❌ Failed to load ABaseEnemy class")
        return False

    # Create Blueprint
    factory = unreal.BlueprintFactory()
    try:
        factory.set_editor_property('target_class', parent_class)
    except Exception as e:
        unreal.log(f"  ⚠️  target_class set failed: {e}, trying ParentClass...")
        try:
            factory.set_editor_property('ParentClass', parent_class)
        except Exception as e2:
            unreal.log(f"  ⚠️  ParentClass also failed: {e2}")

    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=bp_name,
        package_path=bp_dir,
        asset_class=unreal.Blueprint,
        factory=factory,
    )

    if bp is None:
        unreal.log_error(f"  ❌ Failed to create BP_{key}")
        return False

    unreal.EditorAssetLibrary.save_asset(bp_path)
    unreal.log(f"  ✅ Created BP_{key}")
    return True


def configure_default_object(monster: dict) -> bool:
    """Configure the Blueprint's default SkeletalMesh and AnimBP.

    Note: Setting per-instance SkeletalMesh on Blueprint CDO via Python
    is limited. We'll create a config struct for runtime lookup instead.
    """
    key = monster["key"]
    bp_path = f"{monster['content_path']}/Blueprints/BP_{key}"

    if not unreal.EditorAssetLibrary.does_asset_exist(bp_path):
        unreal.log_error(f"  ❌ BP not found: {bp_path}")
        return False

    # Verify all required assets exist
    mesh = unreal.load_asset(monster["mesh_path"])
    abp = unreal.load_asset(monster["abp_path"])

    if mesh is None:
        unreal.log_error(f"  ❌ Mesh not found: {monster['mesh_path']}")
        return False

    unreal.log(f"  ✅ Assets verified for BP_{key}:")
    unreal.log(f"    Mesh: {mesh.get_name()}")
    if abp:
        unreal.log(f"    AnimBP: {abp.get_name()}")
    else:
        unreal.log(f"    ⚠️  AnimBP not found")

    return True


def main():
    unreal.log("=" * 60)
    unreal.log("[ENEMY BPS] Creating Enemy Blueprint Classes")
    unreal.log("=" * 60)

    total = 0
    for monster in MONSTERS:
        key = monster["key"]
        unreal.log(f"\n--- BP_{key} ---")

        if create_enemy_blueprint(monster):
            total += 1
            configure_default_object(monster)

    unreal.log("\n" + "=" * 60)
    unreal.log(f"[ENEMY BPS] Complete: {total}/4 Blueprints created")
    unreal.log("=" * 60)
    unreal.log("")
    unreal.log("Next steps (manual in editor):")
    unreal.log("  1. Open each BP → Class Defaults")
    unreal.log("  2. Set EnemyType enum (CandyZombie/Gingerbread/etc)")
    unreal.log("  3. Set SkeletalMesh in Mesh component")
    unreal.log("  4. Set AnimClass in Mesh component")
    unreal.log("  5. Configure collision capsule")

    unreal.SystemLibrary.execute_console_command(None, 'quit')


main()
