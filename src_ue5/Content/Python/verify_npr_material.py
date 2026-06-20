import unreal
import time
import os
import glob
import shutil

# 使用 EditorLoadingAndSavingUtils 加载关卡
level_path = "/Game/Maps/TestLevel_NPR"
unreal.log(f"[NPR Verify] Loading level: {level_path}")

try:
    unreal.EditorLoadingAndSavingUtils.load_map(level_path)
    unreal.log("[NPR Verify] load_map called successfully")
except AttributeError:
    # UE5.7 可能 API 变了，尝试替代方法
    unreal.log("[NPR Verify] load_map not available, trying asset tools...")
    try:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        asset_tools.open_editor_for_assets([level_path])
    except Exception as e2:
        unreal.log_error(f"[NPR Verify] All load methods failed: {e2}")

# 等待关卡加载和渲染
unreal.log("[NPR Verify] Waiting 15 seconds for level load and render...")
time.sleep(15)

# 列出 Actor
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
unreal.log(f"[NPR Verify] Actors in level: {len(all_actors)}")
for actor in all_actors[:15]:
    cls = actor.get_class().get_name()
    unreal.log(f"  {actor.get_actor_label()} -> {cls}")

# 截图
unreal.log("[NPR Verify] Taking HighResShot...")
unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
time.sleep(5)

# 复制最新截图
try:
    ss_dir = "/home/vipuser/first-game/src_ue5/Saved/Screenshots/LinuxEditor"
    docs_dir = "/home/vipuser/first-game/docs"
    shots = glob.glob(os.path.join(ss_dir, "HighresScreenshot*.png"))
    if shots:
        latest = max(shots, key=os.path.getmtime)
        dest = os.path.join(docs_dir, "npr_gpu_render.png")
        shutil.copy2(latest, dest)
        unreal.log(f"[NPR Verify] Copied: {dest}")
except Exception as e:
    unreal.log_error(f"[NPR Verify] Copy failed: {e}")

unreal.log("[NPR Verify] Complete!")
