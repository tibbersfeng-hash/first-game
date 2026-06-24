"""
cinematic_capture.py
 cinematic camera tour around monsters for video capture
"""
import unreal
import time
import os

OUTPUT_DIR = r"D:\workspace\first-game\docs\video_frames"
TOTAL_FRAMES = 60  # 60 frames at 24fps = 2.5 seconds
RESOLUTION = (1920, 1080)

def setup_camera():
    """创建电影相机"""
    # 删除旧相机
    existing = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in existing:
        if "CineCamera" in actor.get_name() or "CaptureCamera" in actor.get_name():
            unreal.EditorLevelLibrary.destroy_actor(actor)
    
    # 创建新相机
    camera = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CineCameraActor,
        unreal.Vector(0, -800, 300),
        unreal.Rotator(0, 0, 0)
    )
    camera.set_actor_label("CaptureCamera")
    
    # 设置相机参数
    camera_settings = camera.get_cine_camera_component()
    camera_settings.set_editor_property("current_focal_length", 35.0)
    camera_settings.set_editor_property("focus_distance", 1000.0)
    
    return camera

def get_monster_positions():
    """获取怪物位置"""
    positions = []
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if "BaseEnemy" in actor.get_class().get_name() or "Enemy" in actor.get_actor_label():
            pos = actor.get_actor_location()
            positions.append(pos)
            unreal.log(f"Found monster at: {pos}")
    
    # 如果没有找到怪物，使用预设位置
    if not positions:
        positions = [
            unreal.Vector(-500, 0, 0),
            unreal.Vector(500, 0, 0),
            unreal.Vector(0, -500, 0),
            unreal.Vector(0, 500, 0),
        ]
        unreal.log("Using preset monster positions")
    
    return positions

def capture_frame(camera, frame_index, total_frames):
    """在当前位置截图"""
    # 计算环绕角度
    import math
    angle = (frame_index / total_frames) * 2 * math.pi
    
    # 计算相机位置 (环绕中心)
    center = unreal.Vector(0, 0, 200)
    radius = 1200
    height = 400 + 200 * math.sin(frame_index / total_frames * math.pi)
    
    cam_x = center.x + radius * math.cos(angle)
    cam_y = center.y + radius * math.sin(angle)
    cam_z = height
    
    camera.set_actor_location(unreal.Vector(cam_x, cam_y, cam_z))
    
    # 让相机看向中心
    look_at_rot = camera.find_lookat_rotation(center)
    camera.set_actor_rotation(look_at_rot)
    
    # 截图
    filename = f"frame_{frame_index:04d}.png"
    filepath = os.path.join(OUTPUT_DIR, filename)
    
    unreal.EditorLevelLibrary.take_high_res_screenshot(
        RESOLUTION[0], RESOLUTION[1],
        filepath,
        camera=camera,
        include_ui=False
    )
    
    return filepath

def main():
    unreal.log("=== 开始电影化截图 ===")
    
    # 清理之前的截图
    for f in os.listdir(OUTPUT_DIR):
        if f.startswith("frame_") and f.endswith(".png"):
            os.remove(os.path.join(OUTPUT_DIR, f))
    
    # 设置相机
    camera = setup_camera()
    if not camera:
        unreal.log_error("无法创建相机")
        return
    
    # 获取怪物位置
    monsters = get_monster_positions()
    unreal.log(f"找到 {len(monsters)} 个怪物")
    
    # 截图序列
    for i in range(TOTAL_FRAMES):
        filepath = capture_frame(camera, i, TOTAL_FRAMES)
        unreal.log(f"截图 {i+1}/{TOTAL_FRAMES}: {filepath}")
        
        # 等待截图完成
        time.sleep(0.5)
    
    unreal.log("=== 截图完成 ===")
    unreal.log(f"共 {TOTAL_FRAMES} 帧保存到: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
