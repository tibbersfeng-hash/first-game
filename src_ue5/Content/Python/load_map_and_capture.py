"""
load_map_and_capture.py
加载地图并截取多帧画面
"""
import unreal
import time
import os

OUTPUT_DIR = r"D:\workspace\first-game\docs\video_frames"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def main():
    unreal.log("=== 加载地图并截图 ===")
    
    # 加载地图
    map_path = "/Game/Maps/TestLevel_Monsters"
    unreal.log(f"加载地图: {map_path}")
    
    # 使用 EditorLevelLibrary 加载地图
    try:
        unreal.EditorLoadingAndSavingUtils.load_map(map_path)
        unreal.log("地图加载命令已发送")
        
        # 等待地图加载
        time.sleep(10)
        unreal.log("等待地图加载完成...")
    except Exception as e:
        unreal.log_error(f"加载地图失败: {e}")
    
    # 获取相机
    camera = None
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in all_actors:
        if "Camera" in actor.get_class().get_name():
            camera = actor
            break
    
    if not camera:
        # 创建相机
        camera = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.CineCameraActor,
            unreal.Vector(0, -1000, 500),
            unreal.Rotator(-20, 0, 0)
        )
        camera.set_actor_label("CaptureCam")
    
    # 截取 10 帧
    for i in range(10):
        import math
        angle = (i / 10.0) * 2 * math.pi
        radius = 1200
        center = unreal.Vector(0, 0, 200)
        
        cam_x = center.x + radius * math.cos(angle)
        cam_y = center.y + radius * math.sin(angle)
        cam_z = center.z + 400
        
        camera.set_actor_location(unreal.Vector(cam_x, cam_y, cam_z))
        
        # 看向中心
        look_dir = center - unreal.Vector(cam_x, cam_y, cam_z)
        yaw = math.degrees(math.atan2(look_dir.y, look_dir.x))
        pitch = -math.degrees(math.atan2(look_dir.z, math.sqrt(look_dir.x**2 + look_dir.y**2)))
        camera.set_actor_rotation(unreal.Rotator(pitch, yaw, 0))
        
        # 截图
        filename = f"tour_{i:03d}.png"
        filepath = os.path.join(OUTPUT_DIR, filename)
        unreal.EditorLevelLibrary.take_high_res_screenshot(1920, 1080, filepath, camera=camera)
        unreal.log(f"截图 {i+1}/10: {filename}")
        time.sleep(1)
    
    unreal.log("=== 截图完成 ===")

if __name__ == "__main__":
    main()
