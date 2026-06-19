import unreal
import time

unreal.log("[Screenshot] Waiting 15 seconds for game to load...")
time.sleep(15)

unreal.log("[Screenshot] Taking screenshot...")
try:
    # Method 1: Use HighResShot
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    unreal.log("[Screenshot] HighResShot executed")
except Exception as e:
    unreal.log_error(f"[Screenshot] HighResShot failed: {e}")
    try:
        # Method 2: Use shot
        unreal.SystemLibrary.execute_console_command(None, "shot")
        unreal.log("[Screenshot] shot executed")
    except Exception as e2:
        unreal.log_error(f"[Screenshot] shot failed: {e2}")
