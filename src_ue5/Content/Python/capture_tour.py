"""
capture_tour.py - 简化版电影化漫游截图
"""
import unreal
import math
import os
import time

OUTPUT_DIR = r"D:\workspace\first-game\docs\video_frames"
NUM_FRAMES = 30  # 30帧，24fps = 1.25秒
os.makedirs(OUTPUT_DIR, exist_ok=True)

def main():
    unreal.log("=== 开始电影化漫游截图 ===")
    
    # 清理旧截图
    for f in os.listdir(OUTPUT_DIR):
        if f.endswith(".png"):
            try:
                os.remove(os.path.join(OUTPUT_DIR, f))
            except:
                pass
    
    # 获取所有 Actor 找到怪物
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    monsters = []
    for actor in all_actors:
        label = actor.get_actor_label()
        if "Enemy" in label or "BaseEnemy" in str(actor.get_class()):
            monsters.append(actor)
            unreal.log(f"Found: {label} at {actor.get_actor_location()}")
    
    if not monsters:
        unreal.log("No monsters found, using center position")
        center = unreal.Vector(0, 0, 200)
    else:
        # 计算怪物中心
        cx = sum(m.get_actor_location().x for m in monsters) / len(monsters)
        cy = sum(m.get_actor_location().y for m in monsters) / len(monsters)
        cz = sum(m.get_actor_location().z for m in monsters) / len(monsters)
        center = unreal.Vector(cx, cy, cz)
    
    unreal.log(f"Center: {center}, Monsters: {len(monsters)}")
    
    # 创建相机
    camera = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.CineCameraActor,
        unreal.Vector(center.x + 1000, center.y, center.z + 500),
        unreal.Rotator(0, 0, 0)
    )
    camera.set_actor_label("TourCamera")
    
    # 设置视锥
    cam_comp = camera.get_cine_camera_component()
    cam_comp.set_editor_property("current_focal_length", 35.0)
    
    # 截图序列
    for i in range(NUM_FRAMES):
        angle = (i / NUM_FRAMES) * 2 * math.pi
        radius = 1500
        height_offset = 300 * math.sin(angle * 2)  # 上下起伏
        
        cam_x = center.x + radius * math.cos(angle)
        cam_y = center.y + radius * math.sin(angle)
        cam_z = center.z + 400 + height_offset
        
        camera.set_actor_location(unreal.Vector(cam_x, cam_y, cam_z))
        
        # 看向中心
        look_dir = center - unreal.Vector(cam_x, cam_y, cam_z)
        yaw = math.degrees(math.atan2(look_dir.y, look_dir.x))
        dist_xy = math.sqrt(look_dir.x**2 + look_dir.y**2)
        pitch = -math.degrees(math.atan2(look_dir.z, dist_xy))
        
        camera.set_actor_rotation(unreal.Rotator(pitch, yaw, 0))
        
        # 截图
        filename = f"frame_{i:04d}.png"
        filepath = os.path.join(OUTPUT_DIR, filename)
        
        unreal.EditorLevelLibrary.take_high_res_screenshot(
            1920, 1080,
            filepath,
            camera=camera
        )
        
        unreal.log(f"Frame {i+1}/{NUM_FRAMES}: {filename}")
    
    # 清理相机
    unreal.EditorLevelLibrary.destroy_actor(camera)
    
    unreal.log(f"=== 截图完成! {NUM_FRAMES} 帧 ===")

if __name__ == "__main__":
    main()
