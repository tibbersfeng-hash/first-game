"""
UE5 Editor Python 脚本 - 自动导入 Huikong 3D 资产 (UE5.7 最终版)

使用方法:
  在 UE5 Editor 的 Python 控制台执行:
  exec(open(r'/Users/Tibers/game/first-game/tools/import_huikong_assets.py').read())
"""

import unreal
import os

# 配置
HUIKONG_BASE = "/Game/Characters/Huikong"

try:
    FBX_SRC = os.path.join(unreal.Paths.project_dir(), "Content", "_fbx_staging")
except:
    FBX_SRC = "/Users/Tibers/game/first-game/src_ue5/Content/_fbx_staging"

def log(msg):
    unreal.log(f"[Huikong] {msg}")

def refresh_assets():
    """强制刷新资产注册表"""
    try:
        unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(
            ["/Game/Characters/"], recursive=True
        )
    except:
        pass

def ensure_dirs():
    for d in ["Mesh", "Skeleton", "Physics", "Animations", "ABP", "Materials"]:
        path = f"{HUIKONG_BASE}/{d}"
        if not unreal.EditorAssetLibrary.does_directory_exist(path):
            unreal.EditorAssetLibrary.make_directory(path)

def find_or_create_skeleton():
    """查找骨骼，如果不存在则从模型创建"""
    refresh_assets()

    # 优先检查已知路径
    known = f"{HUIKONG_BASE}/Skeleton/SK_Huikong"
    if unreal.EditorAssetLibrary.does_asset_exist(known):
        log(f"找到骨骼: {known}")
        return unreal.load_asset(known)

    # 搜索 Mesh 目录
    mesh_path = f"{HUIKONG_BASE}/Mesh/SKM_Huikong"
    if not unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        log(f"错误: 模型不存在 {mesh_path}")
        return None

    log("未找到独立骨骼，需要从模型提取...")
    log("请在 UE5 中手动操作:")
    log("  1. 双击打开 SKM_Huikong")
    log("  2. 左侧面板找到 Skeleton → 右键 → Save As")
    log("  3. 保存到 /Game/Characters/Huikong/Skeleton/SK_Huikong")
    log("  4. 保存后重新运行此脚本")
    return None

def import_one_animation(en_name, skeleton):
    """导入单个动画"""
    # 查找 FBX 文件
    fbx = None
    for name in [f"{en_name}.fbx"]:
        p = os.path.join(FBX_SRC, name)
        if os.path.exists(p):
            fbx = p
            break

    if not fbx:
        log(f"  跳过 {en_name}: 文件不存在")
        return False

    # 检查是否已导入
    dest = f"{HUIKONG_BASE}/Animations/{en_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(dest):
        log(f"  已存在: {en_name}")
        return True

    log(f"  导入: {os.path.basename(fbx)}")

    task = unreal.AssetImportTask()
    task.filename = fbx
    task.destination_path = f"{HUIKONG_BASE}/Animations"
    task.destination_name = en_name
    task.automated = True
    task.replace_existing = True
    task.save = True

    opts = unreal.FbxImportUI()
    opts.import_mesh = False
    opts.import_animations = True
    opts.import_textures = False
    opts.import_materials = False
    opts.skeleton = skeleton
    opts.override_full_name = True

    task.options = opts

    try:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        refresh_assets()
        if unreal.EditorAssetLibrary.does_asset_exist(dest):
            log(f"  ✓ {en_name}")
            return True
        else:
            log(f"  ✗ {en_name}: 导入后资产不存在")
            return False
    except Exception as e:
        log(f"  ✗ {en_name}: {e}")
        return False

def main():
    log("=" * 50)
    log("导入 Huikong 3D 资产")
    log(f"FBX: {FBX_SRC}")

    if not os.path.exists(FBX_SRC):
        log(f"错误: {FBX_SRC} 不存在")
        return

    # 1. 创建目录
    ensure_dirs()

    # 2. 查找骨骼
    skeleton = find_or_create_skeleton()
    if not skeleton:
        return

    # 3. 导入动画
    log("导入动画...")
    anims = [
        "Anim_Huikong_Idle_01",
        "Anim_Huikong_Idle_02",
        "Anim_Huikong_Walk_01",
        "Anim_Huikong_Run_01",
        "Anim_Huikong_LightAttack_01",
        "Anim_Huikong_HitReaction",
        "Anim_Huikong_Landing",
    ]

    ok = 0
    for name in anims:
        if import_one_animation(name, skeleton):
            ok += 1

    log(f"动画: {ok}/{len(anims)} 成功")
    log("=" * 50)

main()
