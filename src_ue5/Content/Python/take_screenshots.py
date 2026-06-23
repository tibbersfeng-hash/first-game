import unreal
import os
import time

def take_screenshots():
    """截取游戏截图"""
    
    # 截图保存目录
    screenshot_dir = r"D:\workspace\first-game\docs\screenshots"
    os.makedirs(screenshot_dir, exist_ok=True)
    
    unreal.log('=== 开始截图 ===')
    
    # 1. 编辑器截图
    timestamp = time.strftime('%Y%m%d_%H%M%S')
    screenshot_path = os.path.join(screenshot_dir, f'editor_{timestamp}.png')
    
    # 使用编辑器截图功能
    unreal.EditorLevelLibrary().take_high_res_screenshot(
        1920, 1080,
        screenshot_path,
        include_ui=False
    )
    
    unreal.log(f'✅ 编辑器截图: {screenshot_path}')
    
    return screenshot_path

if __name__ == '__main__':
    take_screenshots()
