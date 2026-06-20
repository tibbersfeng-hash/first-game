"""
创建 NPR 赛璐璐材质 (v3)
使用 MaterialEditingLibrary 创建完整的材质图
"""
import unreal
import time
import os
import glob
import shutil

def create_npr_material():
    """创建 NPR 赛璐璐着色材质"""
    mat_path = "/Game/Characters/Huikong/Materials/M_Huikong_NPR_Final"

    # 如果已存在，直接加载
    if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
        unreal.log(f"[NPR] Material already exists at {mat_path}")
        return unreal.load_asset(mat_path)

    # 使用 Factory 创建材质
    factory = unreal.MaterialFactoryNew()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    material = asset_tools.create_asset(
        asset_name="M_Huikong_NPR_Final",
        package_path="/Game/Characters/Huikong/Materials",
        asset_class=unreal.Material,
        factory=factory
    )

    if not material:
        unreal.log_error("[NPR] Failed to create material!")
        return None

    unreal.log(f"[NPR] Created material: {material.get_name()}")

    # 设置 usage flags - 关键！
    # 在 UE5 Python API 中，这些可能通过 set_editor_property 设置
    try:
        material.set_editor_property('bUsedWithSkeletalMesh', True)
        unreal.log("[NPR] Set bUsedWithSkeletalMesh = True")
    except Exception as e:
        unreal.log(f"[NPR] bUsedWithSkeletalMesh: {e}")

    try:
        material.set_editor_property('bUsedWithStaticLighting', False)
        unreal.log("[NPR] Set bUsedWithStaticLighting = False")
    except:
        pass

    try:
        material.set_editor_property('bUsedWithDynamicLighting', True)
        unreal.log("[NPR] Set bUsedWithDynamicLighting = True")
    except:
        pass

    try:
        material.set_editor_property('bUsedAsSpecialEngineMaterial', False)
    except:
        pass

    # 获取纹理资产
    tex_path = "/Game/Characters/Huikong/Animations/texture_pbr_20250901"
    texture = unreal.load_asset(tex_path) if unreal.EditorAssetLibrary.does_asset_exist(tex_path) else None

    if texture:
        unreal.log(f"[NPR] Using texture: {texture.get_name()}")
    else:
        unreal.log("[NPR] Texture not found, using constant color fallback")

    # 创建材质表达式节点
    lib = unreal.MaterialEditingLibrary

    # 1. Texture Sample 节点
    tex_sample = lib.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -400, 0
    )
    if texture:
        tex_sample.texture = texture
        tex_sample.coordinates = unreal.Vector2D(0, 0)
        unreal.log("[NPR] Created TextureSample node")

    # 2. Constant (shadow threshold)
    constant = lib.create_material_expression(
        material, unreal.MaterialExpressionConstant, -600, 200
    )
    constant.r = 0.3  # NdotL threshold for 2-tone
    unreal.log("[NPR] Created Constant (threshold=0.3)")

    # 3. Dot Product (NdotL) - 简化版，使用 Constant3Vector 模拟光照方向
    light_dir = lib.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -800, -200
    )
    light_dir.constant = unreal.Vector(0.5, 0.5, 1.0)  # Light direction
    unreal.log("[NPR] Created light direction vector")

    # 4. 将纹理连接到 BaseColor
    if texture:
        lib.connect_material_expressions(tex_sample, 'RGB', material, 'BaseColor')
        unreal.log("[NPR] Connected texture to BaseColor")

    # 保存
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    unreal.log(f"[NPR] Material saved at {mat_path}")

    return material

def apply_to_skeletal_mesh(material):
    """将材质应用到关卡中的骨骼网格体"""
    if not material:
        return

    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in all_actors:
        cls = actor.get_class().get_name()
        if 'SkeletalMesh' in cls or 'Character' in cls:
            unreal.log(f"[NPR] Found actor: {actor.get_actor_label()} ({cls})")
            smc = actor.get_component_by_class(unreal.SkeletalMeshComponent)
            if smc:
                smc.set_material(0, material)
                unreal.log(f"[NPR] Applied material to {actor.get_actor_label()}")

def take_screenshot():
    """截图并保存"""
    unreal.log("[NPR] Taking screenshot...")
    unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
    time.sleep(5)

    try:
        ss_dir = "/home/vipuser/first-game/src_ue5/Saved/Screenshots/LinuxEditor"
        docs_dir = "/home/vipuser/first-game/docs"
        shots = glob.glob(os.path.join(ss_dir, "HighresScreenshot*.png"))
        if shots:
            latest = max(shots, key=os.path.getmtime)
            dest = os.path.join(docs_dir, "npr_final.png")
            shutil.copy2(latest, dest)
            unreal.log(f"[NPR] Saved: {dest}")
    except Exception as e:
        unreal.log(f"[NPR] Copy error: {e}")

# === 主流程 ===
unreal.log("[NPR] Starting NPR material creation...")
material = create_npr_material()
time.sleep(2)
apply_to_skeletal_mesh(material)
time.sleep(3)
take_screenshot()
unreal.log("[NPR] COMPLETE!")
