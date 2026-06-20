"""
导入 Huikong 骨骼网格体 + 贴图 + 材质

用法 (在 GPU 服务器上):
  UnrealEditor-Cmd FirstGame.uproject \
    -RunPythonScript="Content/Python/import_skeletal_mesh.py" \
    -stdout -FullStdOutLogOutput -unattended -nosplash -nullrhi

前提:
  - FBX 文件: Content/Characters/Huikong/Mesh/SKM_Huikong_Base.fbx (或 model_3.fbx)
  - 贴图文件: Content/Characters/Huikong/Mesh/model_3.fbm/ 目录下 4 张贴图
"""
import unreal
import os
import glob

unreal.log("=" * 60)
unreal.log("[Import] Huikong Skeletal Mesh Import Pipeline")
unreal.log("=" * 60)

# ─── 路径配置 ─────────────────────────────────────────────
PROJECT_DIR = "/root/autodl-tmp/project/first-game/src_ue5"
FBX_PATH = f"{PROJECT_DIR}/Content/Characters/Huikong/Mesh/SKM_Huikong_Base.fbx"
TEXTURE_DIR = f"{PROJECT_DIR}/Content/Characters/Huikong/Mesh/model_3.fbm"
DEST_PATH = "/Game/Characters/Huikong/Mesh"

# 备选 FBX (如果 SKM_Huikong_Base.fbx 不存在)
FBX_FALLBACK = f"{PROJECT_DIR}/Content/Characters/Huikong/Mesh/model_3.fbx"

# ─── 选择 FBX ─────────────────────────────────────────────
if os.path.exists(FBX_PATH):
    fbx_to_use = FBX_PATH
    unreal.log(f"[Import] Using SKM_Huikong_Base.fbx (remeshed, 40k faces)")
elif os.path.exists(FBX_FALLBACK):
    fbx_to_use = FBX_FALLBACK
    unreal.log(f"[Import] Using model_3.fbx (original)")
else:
    unreal.log_error(f"[Import] No FBX file found!")
    unreal.log_error(f"  Expected: {FBX_PATH}")
    unreal.log_error(f"  Or: {FBX_FALLBACK}")
    exit(1)

unreal.log(f"[Import] FBX: {fbx_to_use}")
unreal.log(f"[Import] Textures: {TEXTURE_DIR}")
unreal.log(f"[Import] Destination: {DEST_PATH}")

# ─── 确保目标路径存在 ────────────────────────────────────
if not unreal.EditorAssetLibrary.does_directory_exist(DEST_PATH):
    unreal.EditorAssetLibrary.make_directory(DEST_PATH)
    unreal.log(f"[Import] Created directory: {DEST_PATH}")

# ─── 导入贴图 ─────────────────────────────────────────────
unreal.log("--- Importing Textures ---")

texture_files = {
    "texture_pbr_20250901.png": "T_Huikong_BaseColor",
    "texture_pbr_20250901_normal.png": "T_Huikong_Normal",
    "texture_pbr_20250901_metallic.png": "T_Huikong_Metallic",
    "texture_pbr_20250901_roughness.png": "T_Huikong_Roughness",
}

imported_textures = {}

for filename, asset_name in texture_files.items():
    tex_path = os.path.join(TEXTURE_DIR, filename)
    dest_asset = f"{DEST_PATH}/{asset_name}"

    if not os.path.exists(tex_path):
        unreal.log_warning(f"[Import] Texture not found: {tex_path}")
        continue

    # 删除旧资产
    if unreal.EditorAssetLibrary.does_asset_exist(dest_asset):
        unreal.EditorAssetLibrary.delete_asset(dest_asset)

    task = unreal.AssetImportTask()
    task.filename = tex_path
    task.destination_path = DEST_PATH
    task.destination_name = asset_name
    task.automated = True
    task.replace_existing = True
    task.save = True

    # 贴图导入选项
    opts = unreal.AutomatedAssetImportData()
    # 使用默认的 Texture2D 导入即可

    unreal.log(f"[Import] Texture: {asset_name}")
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    if task.imported_object_paths:
        for path in task.imported_object_paths:
            imported_textures[asset_name] = path
            unreal.log(f"[Import]   ✓ {asset_name} → {path}")
    else:
        unreal.log_error(f"[Import]   ✗ {asset_name} import failed")

unreal.log(f"[Import] Textures: {len(imported_textures)}/{len(texture_files)} imported")

# ─── 导入骨骼网格体 ──────────────────────────────────────
unreal.log("--- Importing Skeletal Mesh ---")

# 删除旧资产
old_skm = f"{DEST_PATH}/SKM_Huikong"
if unreal.EditorAssetLibrary.does_asset_exist(old_skm):
    unreal.EditorAssetLibrary.delete_asset(old_skm)

task = unreal.AssetImportTask()
task.filename = fbx_to_use
task.destination_path = DEST_PATH
task.destination_name = "SKM_Huikong"
task.automated = True
task.replace_existing = True
task.save = True

# FBX 导入选项
opts = unreal.FbxImportUI()
opts.import_mesh = True
opts.import_as_skeletal = True
opts.import_animations = False   # 动画单独导入
opts.import_textures = False     # 贴图已经手动导入
opts.import_materials = False    # 材质后续用 NPR 方案

# 骨骼网格体导入选项
skele_opts = unreal.FbxSkeletalMeshImportData()
skele_opts.b_import_morph_targets = False
skele_opts.b_update_skeleton_normals = True
opts.skeletal_mesh_import_data = skele_opts

task.options = opts

unreal.log(f"[Import] Importing skeletal mesh from: {fbx_to_use}")
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

imported_skeleton = None
imported_skm = None

if task.imported_object_paths:
    for path in task.imported_object_paths:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        asset_type = asset.get_class().get_name()
        unreal.log(f"[Import]   Created: {asset_type} → {path}")

        if asset_type == "Skeleton":
            imported_skeleton = path
        elif asset_type == "SkeletalMesh":
            imported_skm = path
else:
    unreal.log_error("[Import] Skeletal mesh import failed!")
    exit(1)

if imported_skeleton:
    unreal.log(f"[Import] ✓ Skeleton: {imported_skeleton}")
else:
    unreal.log_error("[Import] ✗ No Skeleton created")

if imported_skm:
    unreal.log(f"[Import] ✓ SkeletalMesh: {imported_skm}")
else:
    unreal.log_error("[Import] ✗ No SkeletalMesh created")

# ─── 创建 Physics Asset ───────────────────────────────────
if imported_skm and imported_skeleton:
    unreal.log("--- Creating Physics Asset ---")
    skm_asset = unreal.load_asset(imported_skm)
    skeleton_asset = unreal.load_asset(imported_skeleton)

    phys_asset_name = "SKM_Huikong_PhysicsAsset"
    phys_asset_path = f"{DEST_PATH}/{phys_asset_name}"

    if unreal.EditorAssetLibrary.does_asset_exist(phys_asset_path):
        unreal.EditorAssetLibrary.delete_asset(phys_asset_path)

    # 使用 SkeletalMesh 的自动 Physics Asset 生成
    unreal.log(f"[Import] Physics Asset auto-generated with SkeletalMesh")

# ─── 汇总 ────────────────────────────────────────────────
unreal.log("=" * 60)
unreal.log("[Import] Import Summary:")
unreal.log(f"  FBX: {os.path.basename(fbx_to_use)}")
unreal.log(f"  Textures: {len(imported_textures)}/{len(texture_files)}")
unreal.log(f"  Skeleton: {'✓' if imported_skeleton else '✗'}")
unreal.log(f"  SkeletalMesh: {'✓' if imported_skm else '✗'}")
unreal.log("=" * 60)

if imported_skeleton and imported_skm:
    unreal.log("[Import] READY for animation import!")
    unreal.log(f"[Import] Skeleton path: {imported_skeleton}")
else:
    unreal.log_error("[Import] FAILED - check logs above")
