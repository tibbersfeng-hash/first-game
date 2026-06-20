# UE5 怪物导入与集成指南 — 糖果地牢 4 怪物

> **Created**: 2026-06-20
> **Status**: 管线文档
> **Engine**: Unreal Engine 5.7
> **Scope**: FBX 导入 → 骨架设置 → NPR 材质 → 动画蓝图 → 敌人 BP

---

## 一、概述

本文档描述将 AI 生成 + Blender 修正后的怪物模型导入 UE5 的完整流程，包括：
- FBX 导入设置
- 骨架 (Skeleton) 配置
- NPR 卡通渲染材质实例
- 动画蓝图 (AnimBP) 设置
- 敌人角色蓝图 (Enemy BP) 集成

---

## 二、FBX 导入设置

### 2.1 导入对话框设置

在 UE5 Content Browser 中右键 → Import → 选择 FBX 文件：

| 设置项 | 值 | 说明 |
|--------|-----|------|
| **Mesh** | | |
| Import Mesh | ✅ | 导入网格体 |
| Import Animations | ✅ (动画FBX) / ❌ (静态FBX) | 根据文件类型 |
| Normal Import Method | Import Normals | 使用 FBX 法线 |
| Normal Generation Method | MikkT Space | 标准切线空间 |
| Build Nanite | ❌ | 卡通角色不用 Nanite |
| Remove Degenerates | ✅ | 清理无效面 |
| **Skeleton** | | |
| Import Skeleton | 选择已有或创建新 | 每个怪物独立骨架 |
| Skeleton | (新导入时自动创建) | — |
| **Animation** | | |
| Import Animations | ✅ | 导入动画序列 |
| Animation Length | Exported Time | 使用导出时长 |
| Use Default Sample Rate | ✅ | 30fps |
| **Material** | | |
| Import Materials | ❌ | 不使用 FBX 材质 (用 NPR 替代) |
| Import Textures | ❌ | 后续手动配置 |

### 2.2 导入后验证

导入后立即检查：

```
1. SkeletalMesh 预览：
   - 模型比例正确 (3头身 ≈ 90-120cm 游戏高度)
   - 骨架根骨骼在脚底
   - 无 T-pose 爆炸 (所有顶点正确绑定)

2. 动画序列检查：
   - 每个动画播放流畅
   - Idle 循环无跳帧
   - Walk/Run 循环步态连续
```

---

## 三、骨架 (Skeleton) 配置

### 3.1 UE5 标准骨骼命名

AI 生成的骨骼命名可能不规范。需要映射到 UE5 标准命名：

```
AI 生成骨骼名      →  UE5 标准名
─────────────────────────────────
Root / root        →  Root
Hips / Pelvis      →  pelvis
Spine / spine      →  spine_01
Spine1 / spine1    →  spine_02
Spine2 / spine2    →  spine_03
Neck / neck        →  neck_01
Head / head        →  head
L_Shoulder         →  clavicle_l
L_Arm / L_UpperArm →  upperarm_l
L_Forearm          →  lowerarm_l
L_Hand             →  hand_l
R_Shoulder         →  clavicle_r
R_Arm / R_UpperArm →  upperarm_r
R_Forearm          →  lowerarm_r
R_Hand             →  hand_r
L_Thigh / L_UpLeg  →  thigh_l
L_Calf / L_Leg     →  calf_l
L_Foot             →  foot_l
R_Thigh / R_UpLeg  →  thigh_r
R_Calf / R_Leg     →  calf_r
R_Foot             →  foot_r
```

### 3.2 骨架 Retargeting 设置

如果需要混用动画（如多个怪物共用动画库）：

1. **创建 Retarget Source**:
   - 选择主骨架 → Retarget Manager → 设置为 Retarget Source
   - 选择 Humanoid 骨架类型

2. **配置 IK Retargeter**:
   - Animation → IK Retargeter → 新建
   - Source: 主怪物骨架
   - Target: 目标怪物骨架
   - 配置 Bone Mapping (自动 + 手动微调)

---

## 四、NPR 卡通渲染材质

### 4.1 材质实例创建

基于 `M_AnimeToon_Character` 父材质为每个怪物创建材质实例：

| 怪物 | 材质实例名 | 关键参数 |
|------|-----------|---------|
| 糖果僵尸 | MI_CandyZombie_Body | BaseColor: 绿色, RimColor: 浅绿 |
| 糖果僵尸 | MI_CandyZombie_Apron | BaseColor: 粉色, RimColor: 浅粉 |
| 暴躁姜饼人 | MI_Gingerbread_Body | BaseColor: 棕色, ShadowColor: 深棕 |
| 暴躁姜饼人 | MI_Gingerbread_Frosting | BaseColor: 白色, RimPower: 2.0 |
| 暗影忍者糖 | MI_ShadowNinja_Body | BaseColor: 紫色, Emission: 红(眼睛) |
| 暗影忍者糖 | MI_ShadowNinja_Cape | BaseColor: 深紫, Opacity: 0.85 |
| 铁甲口香糖 | MI_ArmoredGum_Armor | BaseColor: 银灰, Metallic: 0.8 |
| 铁甲口香糖 | MI_ArmoredGum_Shield | BaseColor: 多彩, RimPower: 2.5 |

### 4.2 材质参数配置

```
M_AnimeToon_Character 参数:
├── BaseColor         # 基础色 (RGB)
├── ShadowColor       # 阴影色 (RGB, 比 BaseColor 暗 30-40%)
├── Normal            # 法线贴图 (可选)
├── Roughness         # 粗糙度 (0.3-0.7, 卡通风格偏高)
├── RimPower          # Rim Light 强度 (1.0-3.0)
├── RimColor          # Rim Light 颜色 (通常白色或浅色)
├── OutlineWidth      # 描边宽度 (0.01-0.05)
├── SpecularPower     # 高光锐度 (16-64, 卡通用高值)
└── EmissiveColor     # 自发光 (用于发光眼睛/特效)
```

### 4.3 各怪物材质色值

**糖果僵尸**:
```
Body:
  BaseColor:    (0.2, 0.8, 0.3)     # 亮绿色
  ShadowColor:  (0.1, 0.5, 0.15)    # 深绿色
  RimColor:     (0.6, 1.0, 0.7)     # 浅绿光
  RimPower:     1.5
  Roughness:    0.4

Apron:
  BaseColor:    (1.0, 0.5, 0.65)    # 粉色
  ShadowColor:  (0.7, 0.3, 0.45)    # 深粉
  RimColor:     (1.0, 0.8, 0.85)
  RimPower:     1.2
  Roughness:    0.6
```

**暴躁姜饼人**:
```
Cookie:
  BaseColor:    (0.65, 0.4, 0.2)    # 饼干棕
  ShadowColor:  (0.4, 0.25, 0.1)    # 深棕
  RimColor:     (1.0, 0.85, 0.6)    # 暖光
  RimPower:     1.5
  Roughness:    0.7

Frosting:
  BaseColor:    (0.95, 0.95, 0.9)   # 糖霜白
  ShadowColor:  (0.7, 0.7, 0.65)    # 灰白
  RimColor:     (1.0, 1.0, 0.95)
  RimPower:     2.0
  Roughness:    0.3
```

**暗影忍者糖**:
```
Body:
  BaseColor:    (0.5, 0.3, 0.8)     # 紫色
  ShadowColor:  (0.3, 0.15, 0.6)    # 深紫
  RimColor:     (0.8, 0.6, 1.0)     # 紫光
  RimPower:     2.0
  Roughness:    0.3
  Emissive:     (0.3, 0.0, 0.0)     # 红色微光(眼睛)

Cape:
  BaseColor:    (0.3, 0.15, 0.5)    # 深紫
  ShadowColor:  (0.15, 0.05, 0.35)  # 暗紫
  Opacity:      0.85                 # 半透明
  RimPower:     1.8
  Roughness:    0.4
```

**铁甲口香糖**:
```
Armor:
  BaseColor:    (0.75, 0.78, 0.8)   # 银色
  ShadowColor:  (0.4, 0.42, 0.45)   # 深灰银
  RimColor:     (0.9, 0.92, 0.95)   # 白光
  RimPower:     2.5
  Metallic:     0.8
  Roughness:    0.2

Gum (可见部分):
  BaseColor:    (1.0, 0.5, 0.65)    # 粉色
  ShadowColor:  (0.7, 0.3, 0.45)
  RimColor:     (1.0, 0.8, 0.85)
  RimPower:     1.5
  Roughness:    0.5
```

---

## 五、动画蓝图 (AnimBP) 设置

### 5.1 动画序列导入

每个怪物的动画序列：

| 动画名 | 时长 | 循环 | FBX 来源 |
|--------|------|------|---------|
| AS_Idle | 2-3s | ✅ | motions/待机-1/model_1.fbx |
| AS_Walk | 1-1.5s | ✅ | motions/走路-1/model_1.fbx |
| AS_Jog | 1s | ✅ | motions/慢跑/model_1.fbx |
| AS_Attack | 0.8-1s |  | motions/二连击打/model_1.fbx |
| AS_Hit | 0.3-0.5s | ❌ | motions/受击/model_1.fbx |
| AS_Death | 1-1.5s | ❌ | motions/受击倒地-1/model_1.fbx |

**导入步骤**:
1. 每个 motion FBX 单独导入
2. Import Animations: ✅
3. Skeleton: 选择该怪物的骨架
4. 自动创建 Animation Sequence

### 5.2 BlendSpace 设置

**1D BlendSpace: BS_Locomotion**
- X Axis: Speed (0-600)
- 动画:
  - Speed 0: AS_Idle
  - Speed 150: AS_Walk
  - Speed 400: AS_Jog

**配置**:
```
BlendSpace 1D:
├── X: Speed (0.0 - 600.0)
├── Samples:
│   ├── (0):    AS_Idle     (Loop)
│   ├── (150):  AS_Walk     (Loop)
│   └── (400):  AS_Jog      (Loop)
└── Settings:
    ├── Interpolation: Linear
    └── Extrapolation: Hold Last
```

### 5.3 动画状态机

```
AnimGraph:
└── State Machine: SM_Monster
    ├── Entry → [Idle]
    ├── Idle ──(Speed > 10)──→ [Locomotion]
    ├── Locomotion ──(Speed < 10)──→ [Idle]
    ├── Locomotion ──(Attack Trigger)──→ [Attack]
    ├── Attack ──(Finished)──→ [Idle]
    ├── Any State ──(Hit Trigger)──→ [Hit]
    ├── Hit ──(Finished)──→ [Idle]
    ├── Any State ──(Death Trigger)──→ [Death]
    └── Death → (End)

Locomotion State:
└── BS_Locomotion (BlendSpace 1D)
```

### 5.4 AnimBP 变量

| 变量名 | 类型 | 用途 |
|--------|------|------|
| Speed | Float | 移动速度 (cm/s) |
| IsAttacking | Bool | 攻击中 |
| IsHit | Bool | 受击中 |
| IsDead | Bool | 已死亡 |
| AttackMontage | AnimMontage | 攻击蒙太奇 |

---

## 六、敌人角色蓝图集成

### 6.1 创建敌人 BP

基于 `BP_BaseEnemy` 创建 4 个具体敌人蓝图：

```
BP_CandyZombie     ← extends BP_BaseEnemy
BP_Gingerbread     ← extends BP_BaseEnemy
BP_ShadowNinja     ← extends BP_BaseEnemy
BP_ArmoredGum      ← extends BP_BaseEnemy
```

### 6.2 组件配置

每个敌人 BP 的 SkeletalMeshComponent 设置：

| 设置项 | 值 |
|--------|-----|
| Skeletal Mesh | 对应怪物的 SkeletalMesh |
| Animation Mode | Use Animation Blueprint |
| Anim Class | 对应怪物的 AnimBP |
| Material Override 0 | MI_{Monster}_Body |
| Material Override 1 | MI_{Monster}_Secondary |

### 6.3 碰撞设置

```
Capsule Component:
── Radius: 25-35 cm (根据体型)
├── Half Height: 45-60 cm
└── Collision:
    ├── Preset: Enemy
    ├── Block: WorldDynamic, WorldStatic
    ── Overlap: Visibility, Pawn
```

### 6.4 行为树集成

每个敌人 BP 关联的行为树：

| 怪物 | 行为树 | 特点 |
|------|--------|------|
| 糖果僵尸 | BT_CandyZombie | 慢速追踪，近战攻击 |
| 暴躁姜饼人 | BT_Gingerbread | 中距游走，远程投掷 |
| 暗影忍者糖 | BT_ShadowNinja | 快速突进，近战连击 |
| 铁甲口香糖 | BT_ArmoredGum | 慢速推进，盾牌格挡 |

---

## 七、导入操作清单

### 7.1 首次导入步骤

```
1. 准备文件
   □ 确认 Blender 修正后的 FBX 在 output/3d/{monster}/ 目录
   □ 确认 motions/ 下 6 个动画 FBX 完整

2. UE5 项目设置
   □ 打开 UE5.7 项目
   □ 创建 Content/Monsters/{monster_key}/ 目录结构

3. 导入静态模型
   □ Import model_3.fbx → Content/Monsters/{monster_key}/
   □ 检查骨架和比例
   □ 创建材质实例

4. 导入动画
   □ 逐个导入 6 个 motion FBX
   □ 验证动画播放

5. 创建资源
   □ 创建 BlendSpace (BS_Locomotion)
   □ 创建 AnimBP
   □ 创建 AnimMontage (Attack)
   □ 创建敌人 BP

6. 集成测试
   □ 放置敌人到测试关卡
   □ 验证 AI 行为
   □ 验证动画过渡
   □ 验证 NPR 材质效果
```

### 7.2 Content 目录结构

```
Content/
├── Monsters/
│   ├── CandyZombie/
│   │   ├── Mesh/
│   │   │   └── SK_CandyZombie          # SkeletalMesh
│   │   ├── Materials/
│   │   │   ├── MI_CandyZombie_Body     # 材质实例
│   │   │   └── MI_CandyZombie_Apron
│   │   ├── Animations/
│   │   │   ├── AS_Idle
│   │   │   ├── AS_Walk
│   │   │   ├── AS_Jog
│   │   │   ├── AS_Attack
│   │   │   ├── AS_Hit
│   │   │   └── AS_Death
│   │   ├── BlendSpaces/
│   │   │   └── BS_Locomotion
│   │   ├── AnimBP/
│   │   │   └── ABP_CandyZombie
│   │   └── Blueprints/
│   │       └── BP_CandyZombie
│   ├── Gingerbread/
│   │   └── ... (相同结构)
│   ├── ShadowNinja/
│   │   └── ... (相同结构)
│   └── ArmoredGum/
│       └── ... (相同结构)
```

---

## 八、常见问题

### Q: 导入后模型是 T-pose 爆炸？
**A**: 骨骼绑定权重有问题。回到 Blender 检查 Weight Paint，确保所有顶点都有正确的骨骼权重。

### Q: 动画播放时模型变形？
**A**: 骨骼命名不匹配。检查 FBX 骨骼名和 UE5 骨架骨骼名是否一致，必要时手动映射。

### Q: NPR 材质看起来像写实 PBR？
**A**: 确认使用的是 `M_AnimeToon_Character` 父材质的实例，而非 FBX 自带的材质。检查 ShadowColor 和 RimPower 参数。

### Q: 动画循环有跳帧？
**A**: AI 生成的动画循环可能首尾帧不完全匹配。在 UE5 Animation Editor 中手动调整首尾帧插值。

### Q: 怪物体型比例不对？
**A**: 在 FBX 导入时调整 Global Scale，或在 UE5 SkeletalMesh 的 Preview Scene Settings 中调整。
