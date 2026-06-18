"""
UE5 Editor Python 脚本 - 自动导入 Huikong 3D 资产 (UE5.7 兼容版)

使用方法:
  在 UE5 Editor 的 Python 控制台执行:
  exec(open('/path/to/import_huikong_assets.py').read())
"""

import unreal
import os

# 配置
HUIKONG_BASE = "/Game/Characters/Huikong"

# FBX 源路径
try:
    _proj = unreal.ProjectDirs.project_dir()
except:
    try:
        _proj = unreal.Paths.project_dir()
    except:
        _proj = "/Users/Tibers/game/first-game/src_ue5/"
FBX_SRC = os.path.join(_proj, "Content", "_fbx_staging")

def log(msg):
    unreal.log(f"[Huikong] {msg}")

def ensure_dirs():
    for d in ["Mesh", "Skeleton", "Physics", "Animations", "ABP", "Materials"]:
        path = f"{HUIKONG_BASE}/{d}"
        if not unreal.EditorAssetLibrary.does_directory_exist(path):
            unreal.EditorAssetLibrary.make_directory(path)
            log(f"  创建: {path}")

def import_model():
    """导入武僧模型 (SkeletalMesh, 含骨骼)"""
    fbx = os.path.join(FBX_SRC, "model_3.fbx")
    if not os.path.exists(fbx):
        log(f"错误: 找不到 {fbx}")
        return False

    log(f"导入模型: {fbx}")

    task = unreal.AssetImportTask()
    task.filename = fbx
    task.destination_path = f"{HUIKONG_BASE}/Mesh"
    task.destination_name = "SKM_Huikong"
    task.automated = True
    task.replace_existing = True
    task.save = True

    opts = unreal.FbxImportUI()
    opts.import_mesh = True
    opts.import_textures = False
    opts.import_materials = False
    opts.import_animations = False
    opts.override_full_name = True
    opts.skeleton = None  # 从 FBX 自动创建

    # UE5.7 枚举: unreal.FBXImportType.FBXIT_SKELETAL_MESH / FBXIT_STATIC_MESH / FBXIT_ANIMATION
    opts.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    opts.import_as_skeletal = True

    # 骨骼网格导入设置
    opts.skeletal_mesh_import_data = unreal.FbxSkeletalMeshImportData()
    opts.skeletal_mesh_import_data.import_morph_targets = False
    opts.skeletal_mesh_import_data.update_skeleton_reference_pose = False

    task.options = opts

    try:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        log("  模型导入完成")
        return True
    except Exception as e:
        log(f"  模型导入失败: {e}")
        return False

def find_skeleton():
    """查找已创建的骨骼"""
    candidates = [
        f"{HUIKONG_BASE}/Mesh/SKM_Huikong_Skeleton",
        f"{HUIKONG_BASE}/Skeleton/SK_Huikong",
        f"{HUIKONG_BASE}/Mesh/SK_Huikong",
    ]
    # 也搜索 Mesh 目录下所有 Skeleton 资产
    mesh_assets = unreal.EditorAssetLibrary.list_assets(f"{HUIKONG_BASE}/Mesh", recursive=False)
    for asset_path in mesh_assets:
        if "Skeleton" in asset_path or asset_path.endswith("_Skeleton"):
            candidates.insert(0, asset_path)

    for path in candidates:
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            skel = unreal.load_asset(path)
            log(f"  找到骨骼: {path}")
            return skel, path

    return None, None

def import_animations(skeleton):
    """导入所有动画"""
    anims = [
        ("Anim_Huikong_Idle_01", "待机-1"),
        ("Anim_Huikong_Idle_02", "待机-2"),
        ("Anim_Huikong_Walk_01", "走路-1"),
        ("Anim_Huikong_Run_01", "奔跑"),
        ("Anim_Huikong_LightAttack_01", "回旋踢"),
        ("Anim_Huikong_HitReaction", "受击"),
        ("Anim_Huikong_Landing", "落地"),
    ]

    ok = 0
    for en_name, cn_name in anims:
        # 查找 FBX 文件
        fbx = None
        for candidate in [f"{en_name}.fbx", f"{cn_name}.fbx"]:
            p = os.path.join(FBX_SRC, candidate)
            if os.path.exists(p):
                fbx = p
                break

        if not fbx:
            log(f"  跳过 {en_name}: 文件不存在")
            continue

        log(f"  导入动画: {os.path.basename(fbx)} -> {en_name}")

        task = unreal.AssetImportTask()
        task.filename = fbx
        task.destination_path = f"{HUIKONG_BASE}/Animations"
        task.destination_name = en_name
        task.automated = True
        task.replace_existing = True
        task.save = True

        opts = unreal.FbxImportUI()
        opts.import_mesh = False
        opts.import_textures = False
        opts.import_materials = False
        opts.import_animations = True
        opts.override_full_name = True
        opts.skeleton = skeleton
        opts.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
        opts.import_as_skeletal = False

        opts.anim_sequence_import_data = unreal.FbxAnimSequenceImportData()
        opts.anim_sequence_import_data.import_bone_tracks = True
        opts.anim_sequence_import_data.remove_redundant_keys = True
        opts.anim_sequence_import_data.import_custom_attribute = False

        task.options = opts

        try:
            unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
            log(f"  成功: {en_name}")
            ok += 1
        except Exception as e:
            log(f"  失败 {en_name}: {e}")

    log(f"动画导入: {ok}/{len(anims)} 成功")
    return ok

def move_skeleton(skel_path):
    """把骨骼移到 Skeleton 目录"""
    dest = f"{HUIKONG_BASE}/Skeleton/SK_Huikong"
    if skel_path == dest:
        return dest
    if unreal.EditorAssetLibrary.does_asset_exist(dest):
        return dest
    try:
        unreal.EditorAssetLibrary.rename_asset(skel_path, dest)
        log(f"  骨骼移动到: {dest}")
        return dest
    except:
        return skel_path

def main():
    log("=" * 50)
    log("开始导入 Huikong 3D 资产")
    log(f"FBX 路径: {FBX_SRC}")

    if not os.path.exists(FBX_SRC):
        log(f"错误: 目录不存在!")
        log("请把 FBX 文件放到: src_ue5/Content/_fbx_staging/")
        return

    files = [f for f in os.listdir(FBX_SRC) if f.endswith('.fbx')]
    log(f"找到 {len(files)} 个 FBX: {files}")
    log("=" * 50)

    # 1. 创建目录
    ensure_dirs()

    # 2. 导入模型
    if not import_model():
        return

    # 3. 查找并移动骨骼
    skel, skel_path = find_skeleton()
    if not skel:
        log("未找到骨骼！请手动:")
        log("  1. 在 Content Browser 找到 SKM_Huikong")
        log("  2. 双击打开 -> 查看 Skeleton")
        log("  3. 右键 Skeleton -> Save As -> /Game/Characters/Huikong/Skeleton/SK_Huikong")
        log("  然后重新运行此脚本")
        return

    final_skel_path = move_skeleton(skel_path)
    skeleton = unreal.load_asset(final_skel_path)

    # 4. 导入动画
    import_animations(skeleton)

    log("=" * 50)
    log("导入完成！")
    log("下一步: 创建 Physics Asset + Animation Blueprint + NPR 材质")

main()
