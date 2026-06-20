# Blender 后处理指南 — 糖果地牢 4 怪物

> **Created**: 2026-06-20
> **Status**: 管线文档
> **Scope**: AI 生成模型 → Blender 修正 → UE5 -ready FBX

---

## 一、概述

混元3D AI 生成的模型需要以下后处理才能用于 UE5 游戏：

| 步骤 | 目的 | 时间 |
|------|------|------|
| 拓扑检查/修正 | 修复 AI 生成的非流形几何体 | 15-30 min |
| UV 展开修正 | 确保贴图不拉伸/重叠 | 10-20 min |
| 材质清理 | 统一材质槽，删除多余材质 | 5-10 min |
| 骨骼验证 | 确认绑骨质量，修正权重 | 10-15 min |
| 比例/朝向调整 | 适配 UE5 Mannequin 标准 | 5 min |
| LOD 生成 | 生成低模 LOD1/LOD2 | 10 min |
| 导出 FBX | UE5 兼容格式 | 2 min |

**每个怪物总工时: ~1-1.5 小时**

---

## 二、通用 Blender 后处理流程

### 2.1 导入 AI 生成模型

```python
# Blender Python Console 或 Scripting Tab
import bpy

# 清除默认场景
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete()

# 导入 FBX
bpy.ops.import_scene.fbx(
    filepath="/path/to/model_3.fbx",  # 优先使用 FBX 格式
    global_scale=1.0,
    use_prepost_rot=False,
)
```

### 2.2 拓扑检查清单

在 Edit Mode 下检查：

- [ ] **非流形几何体**: Select > Select All by Trait > Non Manifold
  - 修复: Fill holes (F), Merge vertices by distance
- [ ] **内部面**: Select > Select All by Trait > Interior Faces
  - 修复: Delete faces
- [ ] **重叠顶点**: Mesh > Merge > By Distance (threshold: 0.001m)
- [ ] **翻转法线**: Overlay > Face Orientation (红色=翻转)
  - 修复: Select all > Mesh > Normals > Recalculate Outside
- [ ] **三角面过多**: AI 模型通常全三角面，游戏可用但需检查面数
  - 目标: < 10,000 三角面（怪物）

### 2.3 UV 展开修正

```python
# 检查 UV 重叠
bpy.ops.uv.select_all(action='SELECT')
# 在 UV Editor 中查看是否有重叠

# 自动重新展开（如需要）
bpy.ops.uv.smart_project(angle_limit=66, island_margin=0.02)
```

**目标**:
- UV 岛间距 ≥ 2px (在 1024 贴图下)
- 无拉伸 (Checker texture 测试)
- 对称部位 UV 对称

### 2.4 材质清理

AI 生成的模型通常有 5-15 个材质槽，需要合并为 2-4 个：

| 怪物 | 建议材质槽 |
|------|-----------|
| 糖果僵尸 | Body(绿色), Apron(粉色), Lollipop(多色) |
| 暴躁姜饼人 | Cookie(棕色), Frosting(白色), Star(金色) |
| 暗影忍者糖 | Body(紫色), Cape(深紫), Kunai(粉色晶) |
| 铁甲口香糖 | Armor(银色), Gum(粉色), Shield(多彩), Mace(彩虹) |

**材质合并方法**:
1. Select mesh > Material Properties tab
2. 删除多余材质槽
3. Assign 正确材质到 faces

### 2.5 骨骼验证

```python
# 进入 Pose Mode 检查骨骼
# 关键检查点:
# 1. 骨骼数量是否合理 (< 65 bones for UE5)
# 2. 是否有异常拉伸的顶点权重
# 3. 脚部骨骼是否正确接地
```

**常见问题**:
- 权重过度平滑 → 手动修正 Weight Paint
- 骨骼命名非标准 → 重命名为 UE5 兼容命名
- 根骨骼位置偏移 → 调整到脚底中心

### 2.6 比例和朝向调整

UE5 Mannequin 标准：
- 根骨骼在脚底中心 (0, 0, 0)
- 角色面向 +X (FBX 标准)
- 单位: 1 Blender unit = 1 cm (UE5 默认 1 unit = 1 cm)

```python
# 调整根骨骼位置
armature = bpy.data.objects['Armature']
bpy.context.view_layer.objects.active = armature
bpy.ops.object.mode_set(mode='EDIT')

# 移动根骨骼到原点
root_bone = armature.data.edit_bones['Root']  # 名称可能不同
root_bone.head = (0, 0, 0)
root_bone.tail = (0, 0, 10)  # 10cm 高度

bpy.ops.object.mode_set(mode='OBJECT')
```

### 2.7 LOD 生成

```python
# 使用 Decimate 修改器生成 LOD
# LOD1: 50% 面数
bpy.ops.object.modifier_add(type='DECIMATE')
bpy.context.object.modifiers["Decimate"].ratio = 0.5

# LOD2: 25% 面数
bpy.ops.object.modifier_add(type='DECIMATE')
bpy.context.object.modifiers["Decimate.001"].ratio = 0.25
```

### 2.8 导出 UE5 兼容 FBX

```python
bpy.ops.export_scene.fbx(
    filepath="/path/to/export.fbx",
    use_selection=True,  # 只导出选中对象
    global_scale=1.0,
    apply_unit_scale=True,
    apply_scale_options='FBX_SCALE_ALL',
    object_types={'ARMATURE', 'MESH'},
    use_armature_deform_only=True,
    mesh_smooth_type='FACE',
    add_leaf_bones=False,  # 重要！UE5 不需要叶骨骼
    primary_bone_axis='Y',
    secondary_bone_axis='X',
    bake_anim=True,
    bake_anim_use_all_bones=True,
    bake_anim_use_nla_strips=False,
    bake_anim_use_all_actions=False,
    bake_anim_force_startend_keying=True,
    bake_anim_step=1,
    path_mode='AUTO',
)
```

**关键导出设置**:
- `add_leaf_bones=False` — UE5 会报错
- `primary_bone_axis='Y'` — UE5 骨骼约定
- `bake_anim=True` — 烘焙动画到 FBX
- `object_types={'ARMATURE', 'MESH'}` — 只导出骨架和网格

---

## 三、各怪物专属修正要点

### 3.1 糖果僵尸 (Candy Zombie)

| 问题 | 修正 |
|------|------|
| 眼球悬挂物可能无骨骼 | 添加眼球骨骼 + IK 约束 |
| 棒棒糖天线可能合并到头部 | 分离为独立 mesh + 添加摇摆骨骼 |
| 围裙可能无独立 UV | 分离围裙 faces → 独立 UV 岛 |
| 爪子手指可能融合 | 分离手指 → 添加简单骨骼 |

**特殊动画考虑**:
- 僵尸蹒步态 → Mixamo 的 "Zombie Walk" 动画更合适
- 眼球悬挂 → 需要物理骨骼或顶点动画
- 棒棒糖摇摆 → 简单的骨骼摇摆动画

### 3.2 暴躁姜饼人 (Angry Gingerbread)

| 问题 | 修正 |
|------|------|
| 糖霜装饰可能合并到主体 | 分离装饰 → 独立材质 |
| 投掷姿势手臂可能变形 | 修正手臂权重 |
| 糖果弹可能缺失 | 在 Blender 中建模糖果弹投射物 |
| 星形头部装饰可能低模 | 增加细分 |

**特殊动画考虑**:
- 投掷动画 → Mixamo "Throw" 动画
- 姜饼人走路 → 弹性步伐 (squash & stretch)
- 愤怒表情 → 眉毛骨骼动画

### 3.3 暗影忍者糖 (Shadow Ninja Candy)

| 问题 | 修正 |
|------|------|
| 披风可能无独立骨骼 | 添加披风骨骼链 (3-5 bones) |
| 苦无可能合并到手部 | 分离苦无 → 独立 mesh + 手部骨骼绑定 |
| 半透明材质需单独设置 | 创建透明材质槽 |
| 蹲姿可能需调整为站立 | 修正初始姿势为 T-pose 或 A-pose |

**特殊动画考虑**:
- 忍者蹲伏 → 需要自定义动画
- 披风飘动 → 披风骨骼链 + 物理模拟
- 双持武器 → 左手/右手 IK 约束

### 3.4 铁甲口香糖 (Armored Gum)

| 问题 | 修正 |
|------|------|
| 盔甲板可能合并到身体 | 分离盔甲 → 独立 mesh |
| 泡泡糖头盔可能不透明 | 设置透明材质 (alpha ~0.6) |
| 盾牌可能无独立骨骼 | 添加盾牌骨骼 + 左手 IK |
| 狼牙棒可能合并到右手 | 分离狼牙棒 → 右手骨骼绑定 |
| 体型比例可能需调整 | 确保 2.5 头身比例 |

**特殊动画考虑**:
- 重甲移动 → 缓慢沉重的步伐
- 盾牌格挡 → 防御姿态动画
- 狼牙棒挥舞 → 重型武器动画 (Mixamo "Heavy Attack")

---

## 四、批量处理脚本

以下 Blender Python 脚本可批量处理 4 个怪物的基础修正：

```python
# batch_fix_monsters.py
# 在 Blender Scripting Tab 中运行

import bpy
import os
from pathlib import Path

MONSTERS = {
    "candy_zombie": {
        "fbx": "design/assets/output/3d/candy_zombie/model_3.fbx",
        "materials": {"Body": "green", "Apron": "pink", "Lollipop": "multi"},
    },
    "gingerbread": {
        "fbx": "design/assets/output/3d/gingerbread/model_3.fbx",
        "materials": {"Cookie": "brown", "Frosting": "white", "Star": "gold"},
    },
    "shadow_ninja": {
        "fbx": "design/assets/output/3d/shadow_ninja/model_3.fbx",
        "materials": {"Body": "purple", "Cape": "dark_purple", "Kunai": "pink"},
    },
    "armored_gum": {
        "fbx": "design/assets/output/3d/armored_gum/model_3.fbx",
        "materials": {"Armor": "silver", "Gum": "pink", "Shield": "multi", "Mace": "rainbow"},
    },
}

PROJECT_ROOT = Path(bpy.data.filepath).parent.parent if bpy.data.filepath else Path(".")

def fix_monster(name: str, config: dict):
    """处理单个怪物的基础修正"""
    print(f"\n--- 处理 {name} ---")

    # 清除场景
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()

    # 导入
    fbx_path = str(PROJECT_ROOT / config["fbx"])
    if not os.path.exists(fbx_path):
        print(f"  ❌ FBX 不存在: {fbx_path}")
        return

    bpy.ops.import_scene.fbx(filepath=fbx_path)
    print(f"  ✅ 导入 {fbx_path}")

    # 法线修正
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_recalculate()
    bpy.ops.object.mode_set(mode='OBJECT')
    print(f"  ✅ 法线修正")

    # 重叠顶点合并
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.remove_doubles(threshold=0.001)
    bpy.ops.object.mode_set(mode='OBJECT')
    print(f"  ✅ 重叠顶点合并")

    # 导出修正后 FBX
    output_dir = PROJECT_ROOT / "design" / "assets" / "output" / "3d" / name
    output_dir.mkdir(parents=True, exist_ok=True)
    output_path = str(output_dir / f"{name}_fixed.fbx")

    bpy.ops.export_scene.fbx(
        filepath=output_path,
        use_selection=True,
        global_scale=1.0,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_ALL',
        object_types={'ARMATURE', 'MESH'},
        use_armature_deform_only=True,
        mesh_smooth_type='FACE',
        add_leaf_bones=False,
        primary_bone_axis='Y',
        secondary_bone_axis='X',
        bake_anim=False,  # 动画文件单独处理
        path_mode='AUTO',
    )
    print(f"  ✅ 导出: {output_path}")


# 批量处理
for name, config in MONSTERS.items():
    fix_monster(name, config)

print("\n✅ 批量处理完成！")
```

---

## 五、质量检查清单

每个怪物导出前确认：

### 几何体
- [ ] 三角面数 < 10,000
- [ ] 无非流形几何体
- [ ] 无法线翻转
- [ ] 无内部面

### UV
- [ ] 无 UV 重叠
- [ ] Checker 纹理无拉伸
- [ ] UV 岛间距 ≥ 2px (1024 贴图)

### 骨骼
- [ ] 骨骼数 < 65
- [ ] 根骨骼在脚底 (0,0,0)
- [ ] 无叶骨骼
- [ ] 权重无异常

### 材质
- [ ] 材质槽 ≤ 4 个
- [ ] 材质命名清晰
- [ ] PBR 贴图通道完整 (BaseColor, Normal, Roughness)

### 动画
- [ ] Idle 循环平滑 (首尾帧匹配)
- [ ] Walk/Run 循环步态自然
- [ ] Attack 动画有明显预兆+收招
- [ ] Hit 动画快速反应
- [ ] Death 动画完整倒地

---

## 六、文件命名规范

```
design/assets/output/3d/{monster_key}/
├── model_1.obj          # AI 原始输出
├── model_2.glb          # AI 原始输出
├── model_3.fbx          # AI 原始输出 (绑骨前)
├── preview_*.png        # AI 预览图
├── {monster}_fixed.fbx  # Blender 修正后 (无动画)
├── generation_log.json  # 生成记录
└── motions/
    ├── 待机-1/model_1.fbx       # AI 绑骨动画
    ├── 走路-1/model_1.fbx
    ├── 慢跑/model_1.fbx
    ├── 二连击打/model_1.fbx
    ├── 受击/model_1.fbx
    └── 受击倒地-1/model_1.fbx
```
