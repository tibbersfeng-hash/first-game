import unreal

factory = unreal.AnimBlueprintFactory()
unreal.log("=== AnimBlueprintFactory ===")
for prop in dir(factory):
    if not prop.startswith('_'):
        try:
            val = getattr(factory, prop)
            unreal.log(f"  {prop}: {type(val).__name__}")
        except Exception as e:
            unreal.log(f"  {prop}: (error: {e})")
