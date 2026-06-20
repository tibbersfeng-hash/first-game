"""
验证动画播放 + 添加光照
"""
import unreal
import time
import os
import glob
import shutil

unreal.log("=== Animation + Lighting Setup ===")

# === Step 1: 添加光照 ===
unreal.log("--- Step 1: Adding Directional Light ---")

# 创建 DirectionalLight Actor
light_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.DirectionalLight,
    unreal.Vector(0, 0, 500),
    unreal.Rotator(-45, -45, 0)
)

if light_actor:
    unreal.log(f"Created DirectionalLight: {light_actor.get_actor_label()}")

    # 设置光照参数 — 适合 NPR 的方向光
    light_comp = light_actor.get_component_by_class(unreal.DirectionalLightComponent)
    if light_comp:
        light_comp.set_editor_property('Intensity', 10000.0)
        unreal.log("Set light intensity=10000")

    # 也添加一个 Skylight 作为环境光
    sky_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if sky_actor:
        unreal.log(f"Created SkyLight: {sky_actor.get_actor_label()}")

time.sleep(2)

# === Step 2: 检查动画系统 ===
unreal.log("--- Step 2: Checking Animation System ---")

all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
for actor in all_actors:
    cls = actor.get_class().get_name()
    if 'SkeletalMesh' in cls or 'Character' in cls:
        unreal.log(f"Found: {actor.get_actor_label()} ({cls})")
        smc = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        if smc:
            mesh = smc.get_skinned_asset()
            if mesh:
                unreal.log(f"  Mesh: {mesh.get_name()}")
            anim_bp = smc.get_anim_instance()
            if anim_bp:
                unreal.log(f"  AnimBP class: {anim_bp.get_class().get_name()}")
                # UE5.7 API 检查动画状态
                try:
                    is_playing = anim_bp.is_active()
                    unreal.log(f"  IsActive: {is_playing}")
                except:
                    unreal.log("  (Animation status check not available)")
            else:
                unreal.log("  No AnimInstance (animation may not be set up)")

# === Step 3: 保存关卡 ===
unreal.log("--- Step 3: Saving Level ---")
try:
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("Level saved!")
except Exception as e:
    unreal.log(f"Save error: {e}")

time.sleep(3)

# === Step 4: 截图 ===
unreal.log("--- Step 4: Screenshot ---")
unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
time.sleep(5)

try:
    ss_dir = "/home/vipuser/first-game/src_ue5/Saved/Screenshots/LinuxEditor"
    docs_dir = "/home/vipuser/first-game/docs"
    shots = glob.glob(os.path.join(ss_dir, "HighresScreenshot*.png"))
    if shots:
        latest = max(shots, key=os.path.getmtime)
        dest = os.path.join(docs_dir, "npr_with_light.png")
        shutil.copy2(latest, dest)
        unreal.log(f"Saved: {dest}")
except Exception as e:
    unreal.log(f"Copy error: {e}")

unreal.log("=== COMPLETE ===")
