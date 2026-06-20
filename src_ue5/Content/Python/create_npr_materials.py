"""
Create colored MaterialInstance assets for each monster.
Uses MaterialInstanceConstant with DefaultMaterial as parent,
overriding BaseColor for each monster's signature color.

Triggered via FIRSTGAME_NPR_MATERIALS=1 in init_unreal.py.
"""
import unreal

MONSTERS = [
    {
        "key": "CandyZombie",
        "content_path": "/Game/Monsters/CandyZombie",
        "mesh_path": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
        "base_color": (0.2, 0.8, 0.3, 1.0),   # Green
        "shadow_color": (0.1, 0.5, 0.15, 1.0),
        "rim_color": (0.6, 1.0, 0.7, 1.0),
    },
    {
        "key": "Gingerbread",
        "content_path": "/Game/Monsters/Gingerbread",
        "mesh_path": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
        "base_color": (0.65, 0.4, 0.2, 1.0),   # Brown
        "shadow_color": (0.4, 0.25, 0.1, 1.0),
        "rim_color": (1.0, 0.85, 0.6, 1.0),
    },
    {
        "key": "ShadowNinja",
        "content_path": "/Game/Monsters/ShadowNinja",
        "mesh_path": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
        "base_color": (0.5, 0.3, 0.8, 1.0),    # Purple
        "shadow_color": (0.3, 0.15, 0.6, 1.0),
        "rim_color": (0.8, 0.6, 1.0, 1.0),
    },
    {
        "key": "ArmoredGum",
        "content_path": "/Game/Monsters/ArmoredGum",
        "mesh_path": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
        "base_color": (0.75, 0.78, 0.8, 1.0),  # Silver
        "shadow_color": (0.4, 0.42, 0.45, 1.0),
        "rim_color": (0.9, 0.92, 0.95, 1.0),
    },
]


def ensure_dir(path: str):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_colored_material_instance(monster: dict) -> bool:
    """Create a MaterialInstanceConstant with the monster's base color.

    Uses DefaultMaterial as parent which always has BaseColor parameter.
    """
    key = monster["key"]
    mat_dir = f"{monster['content_path']}/Materials"
    mat_name = f"MI_{key}_NPR"
    mat_path = f"{mat_dir}/{mat_name}"
    ensure_dir(mat_dir)

    if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
        unreal.log(f"  Material {mat_name} already exists, skipping")
        return True

    # Load DefaultMaterial as parent (always has BaseColor)
    parent_mat = unreal.load_asset("/Engine/EngineMaterials/DefaultMaterial")
    if parent_mat is None:
        unreal.log_error(f"  ❌ DefaultMaterial not found")
        return False

    # Create MaterialInstanceConstant
    factory = unreal.MaterialInstanceConstantFactoryNew()
    # Try different property names for parent material
    parent_set = False
    for prop_name in ['Parent', 'parent', 'BaseMaterial', 'base_material', 'SourceMaterial']:
        try:
            factory.set_editor_property(prop_name, parent_mat)
            unreal.log(f"    (factory.{prop_name} = DefaultMaterial)")
            parent_set = True
            break
        except Exception:
            continue

    if not parent_set:
        # Just create the instance without setting parent - it will use default
        unreal.log(f"    ⚠️  Could not set parent material, using default")

    mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=mat_name,
        package_path=mat_dir,
        asset_class=unreal.MaterialInstanceConstant,
        factory=factory,
    )

    if mi is None:
        unreal.log_error(f"  ❌ Failed to create material instance {mat_name}")
        return False

    # Set BaseColor parameter
    r, g, b, a = monster["base_color"]
    try:
        mi.set_editor_property('vector_parameter_values', [
            unreal.MaterialParameterInfo('BaseColor', unreal.MaterialParameterType.SCALAR),
            # Note: This API might not work directly. We'll try alternatives.
        ])
    except Exception:
        pass

    # Try the correct API for setting vector parameters
    try:
        # UE5 Python API for material instance vector parameters
        param_info = unreal.MaterialParameterInfo('Base Color', unreal.MaterialParameterType.VECTOR)
        mi.set_vector_parameter_value('Base Color', unreal.LinearColor(r, g, b, a))
        unreal.log(f"    ✅ Set Base Color = ({r:.2f}, {g:.2f}, {b:.2f})")
    except Exception as e:
        unreal.log(f"    ⚠️  Base Color set failed: {e}")
        # Try alternate parameter name
        try:
            mi.set_vector_parameter_value('BaseColor', unreal.LinearColor(r, g, b, a))
            unreal.log(f"    ✅ Set BaseColor (alt name)")
        except Exception as e2:
            unreal.log(f"    ⚠️  BaseColor also failed: {e2}")

    unreal.EditorAssetLibrary.save_asset(mat_path)
    unreal.log(f"  ✅ Created material instance: {mat_name}")
    return True


def apply_material_to_mesh(monster: dict, mat_instance) -> bool:
    """Apply material instance to the monster's SkeletalMesh."""
    key = monster["key"]
    mesh_path = monster["mesh_path"]

    mesh = unreal.load_asset(mesh_path)
    if mesh is None:
        unreal.log_error(f"  ❌ Mesh not found: {mesh_path}")
        return False

    # Try to set material on the mesh
    try:
        # SkeletalMesh materials can be set via editor property
        mesh.set_editor_property('materials', [mat_instance])
        unreal.EditorAssetLibrary.save_asset(mesh_path)
        unreal.log(f"  ✅ Applied material to SK_{key}")
        return True
    except Exception as e:
        unreal.log(f"  ⚠️  Material apply: {e}")
        return False


def main():
    unreal.log("=" * 60)
    unreal.log("[NPR MATERIALS] Creating Colored Material Instances")
    unreal.log("=" * 60)

    total = 0
    for monster in MONSTERS:
        key = monster["key"]
        unreal.log(f"\n--- {key} ---")

        if create_colored_material_instance(monster):
            total += 1
            # Try to apply to mesh
            mat_path = f"{monster['content_path']}/Materials/MI_{key}_NPR"
            mi = unreal.load_asset(mat_path)
            if mi:
                apply_material_to_mesh(monster, mi)

    unreal.log("\n" + "=" * 60)
    unreal.log(f"[NPR MATERIALS] Complete: {total}/4 material instances")
    unreal.log("=" * 60)
    unreal.log("")
    unreal.log("Note: These are simple colored materials (BaseColor override).")
    unreal.log("Full NPR (2-tone + Rim + Outline) requires GUI editor setup.")

    unreal.SystemLibrary.execute_console_command(None, 'quit')


main()
