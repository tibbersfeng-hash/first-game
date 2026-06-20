import unreal

# Log that we're running in the editor
unreal.log("=" * 60)
unreal.log("[EDITOR CMD] Running in editor context!")
unreal.log("=" * 60)

# List our monster assets
monsters = unreal.EditorAssetLibrary.list_assets("/Game/Monsters", recursive=True)
unreal.log(f"Found {len(monsters)} monster assets")
for a in sorted(monsters)[:10]:
    unreal.log(f"  {a}")

# Check if AnimBPs exist
for abp_path in monsters:
    if "ABP" in abp_path:
        unreal.log(f"  Found AnimBP: {abp_path}")

unreal.log("[EDITOR CMD] Done")
