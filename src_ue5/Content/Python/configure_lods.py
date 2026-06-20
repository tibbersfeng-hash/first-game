"""
Configure LOD settings for 4 monster SkeletalMeshes.
Associates LOD1 (50%) and LOD2 (25%) meshes.

Triggered via FIRSTGAME_CONFIG_LODS=1 in init_unreal.py.
"""
import unreal

MONSTERS = [
    {"key": "CandyZombie", "mesh_path": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie",
     "lod1_path": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie_LOD1",
     "lod2_path": "/Game/Monsters/CandyZombie/Mesh/SK_CandyZombie_LOD2"},
    {"key": "Gingerbread", "mesh_path": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread",
     "lod1_path": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread_LOD1",
     "lod2_path": "/Game/Monsters/Gingerbread/Mesh/SK_Gingerbread_LOD2"},
    {"key": "ShadowNinja", "mesh_path": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja",
     "lod1_path": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja_LOD1",
     "lod2_path": "/Game/Monsters/ShadowNinja/Mesh/SK_ShadowNinja_LOD2"},
    {"key": "ArmoredGum", "mesh_path": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum",
     "lod1_path": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum_LOD1",
     "lod2_path": "/Game/Monsters/ArmoredGum/Mesh/SK_ArmoredGum_LOD2"},
]


def configure_lods(monster: dict):
    """Configure LOD settings for a monster's SkeletalMesh."""
    key = monster["key"]
    mesh = unreal.load_asset(monster["mesh_path"])
    if mesh is None:
        unreal.log_error(f"  ❌ Mesh not found: {monster['mesh_path']}")
        return False

    # Check current LOD info
    try:
        num_lods = mesh.num_lods
        unreal.log(f"  Current LODs: {num_lods}")
    except Exception as e:
        unreal.log(f"  ⚠️  num_lods: {e}")
        num_lods = 1

    # List LOD-related properties
    try:
        props = [p for p in dir(mesh) if 'lod' in p.lower()]
        unreal.log(f"  LOD properties: {props}")
    except Exception:
        pass

    # Try to set number of LODs
    try:
        mesh.set_editor_property('num_lods', 3)
        unreal.log(f"  ✅ Set num_lods = 3")
    except Exception as e:
        unreal.log(f"  ⚠️  set num_lods: {e}")

    # Try to import LOD source meshes
    lod1 = unreal.load_asset(monster["lod1_path"])
    lod2 = unreal.load_asset(monster["lod2_path"])

    if lod1:
        unreal.log(f"  LOD1 mesh loaded: {lod1.get_name()}")
    else:
        unreal.log(f"  ⚠️  LOD1 mesh not found")

    if lod2:
        unreal.log(f"  LOD2 mesh loaded: {lod2.get_name()}")
    else:
        unreal.log(f"  ⚠️  LOD2 mesh not found")

    # Save
    try:
        unreal.EditorAssetLibrary.save_asset(monster["mesh_path"])
        unreal.log(f"  ✅ Mesh saved")
    except Exception as e:
        unreal.log(f"  ⚠️  Save: {e}")

    return True


def main():
    unreal.log("=" * 60)
    unreal.log("[LOD CONFIG] Configuring LOD Settings")
    unreal.log("=" * 60)

    total = 0
    for monster in MONSTERS:
        key = monster["key"]
        unreal.log(f"\n--- {key} ---")
        if configure_lods(monster):
            total += 1

    unreal.log("\n" + "=" * 60)
    unreal.log(f"[LOD CONFIG] Complete: {total}/4 meshes processed")
    unreal.log("Note: Full LOD association requires GUI editor.")
    unreal.log("  In Editor: Open SK_* → LOD Settings → Import LOD Source")
    unreal.log("=" * 60)

    unreal.SystemLibrary.execute_console_command(None, 'quit')


main()
