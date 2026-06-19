import unreal
factory = unreal.AnimBlueprintFactory()
print("=== AnimBlueprintFactory ===")
for prop in dir(factory):
    if not prop.startswith('_'):
        try:
            val = getattr(factory, prop)
            print(f"  {prop}: {type(val).__name__}")
        except:
            print(f"  {prop}: (error)")
