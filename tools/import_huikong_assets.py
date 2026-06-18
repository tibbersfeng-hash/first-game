"""
UE5 Editor Python 脚本 - 自动导入 Huikong 3D 资产

使用方法:
  在 UE5 Editor 的 Python 控制台执行:
  exec(open('/path/to/import_huikong_assets.py').read())
"""

import unreal
import os

# 配置
HUIKONG_BASE_PATH = "/Game/Characters/Huikong"
FBX_SOURCE_PATH = "/root/autodl-tmp/project/first-game/design/assets/output/3d/huikong"

def log(msg):
    unreal.log(f"[Huikong Import] {msg}")

def create_directories():
    """创建目录结构"""
    log("创建目录结构...")

    dirs = [
        f"{HUIKONG_BASE_PATH}/Mesh",
        f"{HUIKONG_BASE_PATH}/Skeleton",
        f"{HUIKONG_BASE_PATH}/Physics",
        f"{HUIKONG_BASE_PATH}/Animations",
        f"{HUIKONG_BASE_PATH}/ABP",
        f"{HUIKONG_BASE_PATH}/Materials",
    ]

    for dir_path in dirs:
        if not unreal.EditorAssetLibrary.does_directory_exist(dir_path):
            unreal.EditorAssetLibrary.make_directory(dir_path)
            log(f"  创建: {dir_path}")

    log("目录创建完成")

def import_base_mesh():
    """导入基础模型"""
    log("导入基础模型...")

    fbx_path = f"{FBX_SOURCE_PATH}/model_3.fbx"
    dest_path = f"{HUIKONG_BASE_PATH}/Mesh/SM_Huikong"

    if not os.path.exists(fbx_path):
        log(f"错误: FBX 文件不存在: {fbx_path}")
        return False

    # 创建导入任务
    task = unreal.AssetImportTask()
    task.filename = fbx_path
    task.destination_path = f"{HUIKONG_BASE_PATH}/Mesh"
    task.destination_name = "SM_Huikong"
    task.automated = True
    task.replace_existing = True
    task.save = True

    # FBX 导入选项
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_textures = False
    options.import_materials = False
    options.import_animations = False
    options.override_full_name = True
    options.mesh_type_to_import = unreal.FBXMeshTypeToImport.STATIC_MESH

    # 骨骼设置
    options.skeleton = None  # 稍后创建

    # 静态网格设置
    options.static_mesh_import_data = unreal.FbxStaticMeshImportData()
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.auto_generate_collision = True
    options.static_mesh_import_data.remove_degenerates = True

    task.options = options

    # 执行导入
    try:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        log(f"  成功: {dest_path}")
        return True
    except Exception as e:
        log(f"  失败: {e}")
        return False

def import_animations():
    """导入动画文件"""
    log("导入动画文件...")

    animations = [
        ("待机-1", "Anim_Huikong_Idle_01"),
        ("待机-2", "Anim_Huikong_Idle_02"),
        ("走路-1", "Anim_Huikong_Walk_01"),
        ("奔跑", "Anim_Huikong_Run_01"),
        ("回旋踢", "Anim_Huikong_LightAttack_01"),
        ("受击", "Anim_Huikong_HitReaction"),
        ("落地", "Anim_Huikong_Landing"),
    ]

    success_count = 0

    for cn_name, en_name in animations:
        fbx_path = f"{FBX_SOURCE_PATH}/motions/{cn_name}/model_1.fbx"
        dest_path = f"{HUIKONG_BASE_PATH}/Animations/{en_name}"

        if not os.path.exists(fbx_path):
            log(f"  跳过: {fbx_path} (不存在)")
            continue

        # 创建导入任务
        task = unreal.AssetImportTask()
        task.filename = fbx_path
        task.destination_path = f"{HUIKONG_BASE_PATH}/Animations"
        task.destination_name = en_name
        task.automated = True
        task.replace_existing = True
        task.save = True

        # FBX 导入选项
        options = unreal.FbxImportUI()
        options.import_mesh = False
        options.import_textures = False
        options.import_materials = False
        options.import_animations = True
        options.override_full_name = True
        options.animation_type = unreal.FBXAnimationTypeToImport.ANIMATION

        # 骨骼设置 - 需要先导入模型并创建骨骼
        # 这里假设骨骼已经存在
        skeleton_path = f"{HUIKONG_BASE_PATH}/Skeleton/SK_Huikong"
        if unreal.EditorAssetLibrary.does_asset_exist(skeleton_path):
            options.skeleton = unreal.load_asset(skeleton_path)
        else:
            log(f"  警告: 骨骼不存在: {skeleton_path}")
            log(f"  请先导入基础模型并创建骨骼")
            continue

        # 动画导入设置
        options.animation_import_data = unreal.FbxAnimSequenceImportData()
        options.animation_import_data.import_custom_attribute = False
        options.animation_import_data.import_bone_tracks = True
        options.animation_import_data.remove_redundant_keys = True

        task.options = options

        # 执行导入
        try:
            unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
            log(f"  成功: {dest_path}")
            success_count += 1
        except Exception as e:
            log(f"  失败: {e}")

    log(f"动画导入完成: {success_count}/{len(animations)} 成功")
    return success_count == len(animations)

def create_skeleton():
    """创建骨骼（如果基础模型导入时未自动创建）"""
    log("检查骨骼...")

    skeleton_path = f"{HUIKONG_BASE_PATH}/Skeleton/SK_Huikong"

    if unreal.EditorAssetLibrary.does_asset_exist(skeleton_path):
        log(f"  骨骼已存在: {skeleton_path}")
        return True

    # 尝试从导入的网格体创建骨骼
    mesh_path = f"{HUIKONG_BASE_PATH}/Mesh/SM_Huikong"
    if not unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        log(f"  错误: 网格体不存在: {mesh_path}")
        log(f"  请先导入基础模型")
        return False

    log("  需要从基础模型创建骨骼")
    log("  请在 UE5 Editor 中手动操作:")
    log("    1. 打开 SM_Huikong")
    log("    2. Asset Actions → Create Skeleton")
    log("    3. 保存为 SK_Huikong")

    return False

def main():
    log("=" * 60)
    log("开始导入 Huikong 3D 资产")
    log("=" * 60)

    # 1. 创建目录
    create_directories()

    # 2. 导入基础模型
    if not import_base_mesh():
        log("基础模型导入失败，终止")
        return

    # 3. 创建骨骼
    if not create_skeleton():
        log("骨骼创建失败，终止")
        return

    # 4. 导入动画
    import_animations()

    log("=" * 60)
    log("导入完成！")
    log("=" * 60)
    log("下一步:")
    log("  1. 创建 Physics Asset (PA_Huikong)")
    log("  2. 创建 Animation Blueprint (ABP_Huikong)")
    log("  3. 创建 NPR 材质 (M_Huikong_Toon)")

if __name__ == "__main__":
    main()
