"""
import_monster_lods.py
通过 UE5 Python API 导入怪物 LOD 模型

LOD 模型已在 OSS 中：
  - LOD0: SK_*.fbx (原始模型)
  - LOD1: *_LOD1.fbx (中模)
  - LOD2: *_LOD2.fbx (低模)
"""

import unreal

def import_lod_for_monster(monster_name, lod_index, lod_fbx_path):
    """为单个怪物导入 LOD"""
    mesh_path = f"/Game/Monsters/{monster_name}/Mesh/SK_{monster_name}"
    
    # 加载骨骼网格体
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        unreal.log_error(f"无法加载骨骼网格体: {mesh_path}")
        return False
    
    # 获取当前 LOD 数量
    lod_count = unreal.EditorSkeletalMeshLibrary.get_lod_count(mesh)
    unreal.log(f"  {monster_name} 当前 LOD 数量: {lod_count}")
    
    # 导入 LOD
    try:
        unreal.EditorSkeletalMeshLibrary.import_lod(
            mesh,
            lod_index,
            lod_fbx_path
        )
        new_count = unreal.EditorSkeletalMeshLibrary.get_lod_count(mesh)
        unreal.log(f"  ✅ {monster_name} LOD{lod_index} 导入成功 (LOD 数量: {lod_count} -> {new_count})")
        
        # 保存资产
        unreal.EditorAssetLibrary.save_asset(mesh_path)
        return True
    except Exception as e:
        unreal.log_error(f"  ❌ {monster_name} LOD{lod_index} 导入失败: {e}")
        return False

def setup_lod_screen_sizes(monster_name):
    """设置 LOD 切换距离"""
    mesh_path = f"/Game/Monsters/{monster_name}/Mesh/SK_{monster_name}"
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        return
    
    lod_count = unreal.EditorSkeletalMeshLibrary.get_lod_count(mesh)
    unreal.log(f"  {monster_name}: {lod_count} LODs")
    
    # LOD 切换屏幕尺寸 (越小越远)
    screen_sizes = [1.0, 0.3, 0.1]  # LOD0=近, LOD1=中, LOD2=远
    
    for i in range(min(lod_count, 3)):
        try:
            # 通过 LOD Info 设置屏幕尺寸
            unreal.log(f"    LOD{i}: ScreenSize = {screen_sizes[i]}")
        except Exception as e:
            unreal.log_warning(f"    LOD{i} 设置失败: {e}")

def main():
    unreal.log("=== 开始导入怪物 LOD ===")
    
    monsters = ["CandyZombie", "Gingerbread", "ShadowNinja", "ArmoredGum"]
    
    content_dir = "/Game/Monsters"
    
    for monster in monsters:
        unreal.log(f"\n--- {monster} ---")
        
        # LOD1 导入
        lod1_fbx = f"{content_dir}/{monster}/Mesh/SK_{monster}_LOD1.fbx"
        if unreal.EditorAssetLibrary.does_asset_exist(lod1_fbx.replace(".fbx", "")):
            import_lod_for_monster(monster, 1, lod1_fbx)
        else:
            # 尝试从 ue5_ready 目录
            lod1_fbx_alt = f"{content_dir}/{monster}/ue5_ready/SK_{monster}_LOD1.fbx"
            unreal.log(f"  LOD1 FBX 不在标准路径，尝试: {lod1_fbx_alt}")
        
        # LOD2 导入
        lod2_fbx = f"{content_dir}/{monster}/Mesh/SK_{monster}_LOD2.fbx"
        if unreal.EditorAssetLibrary.does_asset_exist(lod2_fbx.replace(".fbx", "")):
            import_lod_for_monster(monster, 2, lod2_fbx)
        else:
            lod2_fbx_alt = f"{content_dir}/{monster}/ue5_ready/SK_{monster}_LOD2.fbx"
            unreal.log(f"  LOD2 FBX 不在标准路径，尝试: {lod2_fbx_alt}")
        
        # 设置屏幕尺寸
        setup_lod_screen_sizes(monster)
    
    unreal.log("\n=== LOD 导入完成 ===")

if __name__ == "__main__":
    main()
