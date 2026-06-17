# P0 原型准备指南 — 格斗萌主 3D 美术管线

> **Created**: 2026-06-17
> **目标**: 3 周内产出第一个可动 Q版角色 + 基础动画 + NPR 渲染测试
> **角色**: 气功小武僧 (Huikong) — 最适合作为首发原型

---

## 📋 总览

```
Week 1: 工具安装 + 风格探索 + Blender 基础
Week 2: 角色建模 + Mixamo 绑定 + 基础动画
Week 3: 战斗动画 + UE5 NPR 测试 + P0 评审
```

---

## 🛠️ 工具安装清单

### 必需工具（本地电脑）

| 工具 | 用途 | 平台 | 安装方式 |
|---|---|---|---|
| **Blender 4.x** | 3D 建模/绑定/动画 | Win/Mac/Linux | [官网下载](https://www.blender.org/download/) |
| **Mixamo 账户** | 自动绑定+动画库 | Web | [mixamo.com](https://www.mixamo.com/) (Adobe 账户) |
| **UE5.7** | 游戏引擎 + NPR | Win/Linux | 云服务器已装，本地可选装 |

### 可选工具

| 工具 | 用途 | 何时需要 |
|---|---|---|
| **Meshy.ai** | AI 3D 生成参考模型 | 第 1 周风格探索 |
| **Substance Painter** | 贴图绘制 | 需要复杂贴图时 |
| **Aseprite / Photoshop** | 手绘贴图/UI | 贴图细节调整 |
| **Git LFS** | 大文件版本控制 | 资产版本管理 |

### 安装步骤

#### Blender 4.x

```
1. 访问 https://www.blender.org/download/
2. 下载最新稳定版 (4.3+)
3. 安装后启动，确认版本
4. 推荐设置:
   - Edit → Preferences → Interface → Language: English
   - 安装 "Node Wrangler" 内置插件 (材质编辑必备)
   - 安装 "Import-Export: FBX format" 插件
```

#### Mixamo

```
1. 访问 https://www.mixamo.com/
2. 用 Adobe 账户登录（免费注册）
3. 熟悉界面:
   - Characters: 预制角色（不用，我们自己做）
   - Animations: 动画库（核心资源）
   - Upload: 上传自己的模型绑定
```

#### Meshy.ai (可选)

```
1. 访问 https://www.meshy.ai/
2. 注册试用账号（有免费额度）
3. 熟悉 Text to 3D 和 Image to 3D 功能
```

---

## 📁 项目结构

### 本地目录结构

```
first-game/
├── production/p0-prototype/
│   ├── P0-prototype-guide.md          ← 本文件
│   ├── blender-templates/
│   │   └── firstgame_character.blend  ← Blender 模板项目
│   ├── references/
│   │   ├── chibi-proportions.md       ← Q版比例参考
│   │   ├── huikong-design.md          ← 武僧设计参考
│   │   └── npr-reference.md           ← NPR 渲染参考
│   ├── exports/                        ← Blender 导出目录
│   │   ├── huikong_mesh.fbx           ← 网格
│   │   ├── huikong_rig.fbx            ← 绑定后
│   │   └── animations/                ← Mixamo 动画
│   └── reviews/                        ← 每周评审记录
│
└── design/art/
    ├── art-pipeline-3d-evaluation.md   ← 管线评估
    └── output/huikong/                 ← 已有 2D 参考图
```

### 云服务器目录（恢复后）

```
/root/project/FirstGame/
├── Content/
│   ├── Characters/Huikong/
│   │   ├── Mesh/                       ← FBX 导入的 SkeletalMesh
│   │   ├── Materials/                  ← NPR 材质
│   │   ├── Animations/                 ← 动画资产
│   │   └── Blueprints/                 ← ABP, AnimBP
│   └── Test/
│       └── P0_TestLevel.umap           ← P0 测试关卡
```

---

## 🎯 第一周详细任务

### Day 1-2: 安装 + Blender 基础

**目标**: 熟悉 Blender 基本操作

| 任务 | 时长 | 资源 |
|---|---|---|
| 安装 Blender + Mixamo | 1h | — |
| Blender 基础教程 (界面/导航) | 2h | [Blender 官方教程](https://docs.blender.org/manual/en/latest/getting_started/index.html) |
| 基础建模练习 (立方体→简单角色) | 3h | [Blender Guru Donut Tutorial](https://www.blender.org/support/tutorials/) |

**Day 2 结束交付**:
- [x] Blender 可正常启动
- [x] 能基本导航 (旋转/平移/缩放视图)
- [x] 能创建和编辑基本几何体
- [x] Mixamo 账户已注册

### Day 3-4: Q版角色风格探索

**目标**: 确定 Q版三头身比例，生成参考

| 任务 | 时长 | 资源 |
|---|---|---|
| 在 Blender 中搭建 Q版比例参考 | 2h | 见下方"Q版比例指南" |
| Meshy.ai 生成 5 个 Q版 草稿 | 2h | Meshy Text to 3D |
| 选最佳草稿，导入 Blender 分析 | 1h | — |
| 收集武僧设计参考图 | 1h | 已有 2D sprite + 网络参考 |

**Day 4 结束交付**:
- [x] Blender 中有 Q版比例参考模型
- [x] 选定 1 个风格方向
- [x] 武僧设计参考图收集完毕

### Day 5-7: 开始武僧建模

**目标**: 完成武僧基础模型（block-out）

| 任务 | 时长 | 资源 |
|---|---|---|
| 建模 block-out（低模，大致比例） | 6h | 参考 Blender 角色建模教程 |
| 检查三头身比例 | 1h | 对照参考图 |
| 导出 FBX 测试 | 1h | 导入 UE5 检查尺寸 |

**Day 7 结束交付**:
- [x] 武僧 block-out 模型完成
- [x] 三头身比例正确
- [x] FBX 可导出

---

## 📐 Q版比例指南

### 三头身比例标准

```
┌─────────────┐ ─┐
│             │  │
│   头部      │  1 头高 (含头发)
│             │ ─┤
├─────────────┤  │
│             │  │
│  躯干+手臂  │  1 头高
│             │ ─┤
├─────────────┤  │
│             │  │
│  腿+脚      │  1 头高
│             │ ─┘
└─────────────┘

总高 = 3 头高
```

### Blender 中的比例参考

| 部位 | 比例 (以头高=1) | Blender 单位 (cm) |
|---|---|---|
| 头 (含头发) | 1.0 | 40cm |
| 躯干 | 0.6 | 24cm |
| 手臂长度 | 0.6 | 24cm |
| 腿 | 0.8 | 32cm |
| 脚 | 0.2 | 8cm |
| **总高** | **3.0** | **120cm** |

### 武僧特殊比例

| 部位 | 武僧特点 |
|---|---|
| 头部 | 光头 + 念珠头饰，略圆 |
| 躯干 | 短壮（武僧体格），穿蓝色僧衣 |
| 手臂 | 略粗（习武），戴拳套 |
| 腿部 | 短而有力 |
| 整体轮廓 | 圆轮廓，无锐利边缘 |

---

## 🎨 武僧设计参考

### 视觉特征 (参考已有设计)

```
主色: 蓝 #6699FF
辅色: 金/白
武器: 念珠 + 气功拳套
轮廓: 光头 + 飘带 + 圆轮廓
特效: 金色气功波 + 圆形冲击波
糖果色: 蓝色冰糖
```

### 动画列表 (P0 必需)

| 动画 | 优先级 | 来源 | 时长 |
|---|---|---|---|
| Idle | P0 | Mixamo | 2s 循环 |
| Walk | P0 | Mixamo | 1s 循环 |
| Run | P0 | Mixamo | 0.8s 循环 |
| Jump/Fall/Land | P0 | Mixamo | 各 0.5s |
| Light Attack × 3 | P0 | 手工制作 | 各 1s |
| Heavy Attack | P0 | 手工制作 | 1.5s |
| Skill (气功波) | P0 | 手工制作 | 1.5s |
| Dodge | P0 | Mixamo 修改 | 0.5s |
| Hit Reaction | P0 | Mixamo 修改 | 0.3s |

### Mixamo 动画选择指南

搜索关键词推荐:

| 需要的动画 | Mixamo 搜索词 |
|---|---|
| Idle | "idle", "standing idle", "breathing idle" |
| Walk | "walk", "casual walk" |
| Run | "run", "jog" |
| Jump | "jump", "jump forward" |
| Punch (轻攻) | "punch", "jab", "quick punch" |
| Heavy Punch | "heavy punch", "uppercut", "power punch" |
| Dodge | "dodge", "evasive", "roll" |
| Hit | "hit reaction", "hurt", "stumble" |
| Death | "death", "defeat", "collapse" |

---

## 🔌 Mixamo 工作流

### 绑定流程

```
1. 在 Blender 完成模型
2. 导出为 FBX (T-Pose, 仅网格)
3. 上传到 Mixamo (mixamo.com → Upload Character)
4. Auto-Rig (Mixamo 自动识别关节位置)
   - 调整下巴、手腕、膝盖、手肘位置
   - 确认绿色标记位置正确
5. 绑定完成，下载 Rigged FBX
```

### 动画应用流程

```
1. Mixamo → Animations → 搜索动画
2. 预览动画，调整参数:
   - 动画速度 (Speed)
   - 手臂/腿部过度 (Arm/Leg Over-exaggerate)
   - 手臂/腿部扭曲 (Arm/Leg Twist)
   - 镜像 (Mirror)
3. 选择 "Without Skin" 下载 (仅动画)
4. 导入 Blender，应用到已绑定角色
5. 微调 (调整 Q版 比例下的动画幅度)
6. 导出 FBX (含骨骼+动画)
```

### 动画 retarget 到 UE5

```
1. UE5 导入 Rigged Character FBX → SkeletalMesh
2. 使用 IK Retargeter:
   - Source: Mixamo Skeleton
   - Target: 我们的 Skeleton
3. 批量 retarget 所有动画
4. 在 UE5 Animation Blueprint 中组织
```

---

## 🎮 UE5 NPR 渲染准备

### 材质基础设置 (Toon Shader)

```
UE5 卡通渲染核心节点:
├── 基础色 (Base Color)
├── 阴影色 (Shadow Color) — 通过 Lightmap UV 或 Light 控制
├── 描边 (Outline) — 两种方法:
│   ├── 方法 A: Inverted Hull (反向法线挤出)
│   └── 方法 B: PostProcess (后处理描边)
├── 高光 (Rim Light) — Fresnel 节点
└── 自发光 (Emission) — 技能特效
```

### 测试步骤

```
1. UE5 中新建 Material: M_Huikong_Toon
2. 基础设置:
   - Shading Model: Unlit (或 Subsurface 如果效果需要)
   - 使用 Step 节点实现 2-tone shading
3. 描边:
   - 创建第二个材质 M_Huikong_Outline
   - 复制 Mesh，反向法线，黑色材质，稍大一点
4. Rim Light:
   - Fresnel 节点 + Power 参数
5. 导入武僧模型测试
```

### 参考截图收集

建议收集的参考 (游戏截屏):
- 崩坏3 — 角色材质/描边/高光
- 原神 — 卡通渲染 NPR
- 绝区零 — 高速动作特效
- 龙珠Z Kakarot — 气功波特效
- 罪恶装备 -Strive- — 顶级 2.5D 卡通渲染

---

## 📦 资产规范

### 网格规范

| 项目 | 规格 | 说明 |
|---|---|---|
| 顶点数 | < 15,000 三角面 | Q版角色不需要太高精度 |
| 骨骼数 | < 60 bones | Mixamo 标准骨骼 |
| UV 通道 | 2 个 (UV0 贴图 + UV1 光照) | NPR 可能需要额外通道 |
| 法线 | 统一朝外，平滑组正确 | 避免渲染问题 |

### 贴图规范

| 贴图 | 尺寸 | 格式 | 说明 |
|---|---|---|---|
| Base Color | 2048×2048 | PNG/TGA | 主色贴图 |
| Shadow Color | 1024×1024 | PNG | 阴影色贴图 (可选) |
| Normal Map | 1024×1024 | PNG | 如果需要细节 |
| Emission | 1024×1024 | PNG | 自发光 (技能发光区域) |

### 骨骼命名 (Mixamo 标准)

```
Mixamo 骨骼名 (UE5 兼容):
- mixamorig:Hips
- mixamorig:Spine, Spine1, Spine2
- mixamorig:Neck, Head
- mixamorig:LeftShoulder, LeftArm, LeftForeArm, LeftHand
- mixamorig:RightShoulder, RightArm, RightForeArm, RightHand
- mixamorig:LeftUpLeg, LeftLeg, LeftFoot, LeftToeBase
- mixamorig:RightUpLeg, RightLeg, RightFoot, RightToeBase
```

### 动画规范

| 项目 | 规格 |
|---|---|
| 帧率 | 30fps (UE5 默认) |
| 格式 | FBX 2020 |
| 关键帧 | 只导出关键帧，不烘焙每帧 |
| 循环动画 | Idle/Walk/Run 标记为 Loop |

---

## 📅 P0 里程碑检查

### Week 1 结束 (Day 7)

- [ ] Blender 安装完成，基础操作熟练
- [ ] Mixamo 账户注册完成
- [ ] Q版比例在 Blender 中确定
- [ ] 武僧 block-out 模型完成
- [ ] FBX 导出测试成功

### Week 2 结束 (Day 14)

- [ ] 武僧精模完成
- [ ] 贴图绘制完成（至少 Base Color）
- [ ] Mixamo 绑定成功
- [ ] 基础动画导入 (idle/walk/run/jump)
- [ ] UE5 导入测试，尺寸正确

### Week 3 结束 (Day 21)

- [ ] 战斗动画完成 (light×3, heavy, skill)
- [ ] UE5 NPR 材质测试
- [ ] Animation Blueprint 基础搭建
- [ ] P0 可玩原型（武僧可在测试关卡移动+攻击）
- [ ] P0 评审文档

---

## 🔍 参考资源

### Blender 教程

- [Blender 官方手册](https://docs.blender.org/manual/en/latest/)
- [Blender Guru - Donut Tutorial](https://www.blender.org/support/tutorials/) (入门)
- [CG Fast Tracker - 角色建模教程](https://cgfasttracker.com/) (角色)
- [Q版角色建模关键词](https://www.youtube.com/results?search_query=blender+chibi+character+tutorial) (YouTube 搜索 "blender chibi character tutorial")

### NPR / 卡通渲染

- [UE5 Toon Shader 教程](https://www.youtube.com/results?search_query=unreal+engine+5+toon+shader) (YouTube)
- [UE5 Stylized Rendering 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/stylized-rendering-in-unreal-engine---overview)
- [崩坏3 渲染技术分析](https://zhuanlan.zhihu.com/p/xxx) (知乎搜索)

### Mixamo

- [Mixamo 官方文档](https://helpx.adobe.com/mixamo/user-guide.html)
- [Mixamo to UE5 工作流](https://www.youtube.com/results?search_query=mixamo+to+unreal+engine+retargeting)

### Q版角色参考

- 崩坏3 角色截屏 (游戏内)
- 原神 角色截屏 (游戏内)
- [Pinterest: Chibi Character Design](https://www.pinterest.com/search/pins/?q=chibi+character+design+3d)
- [ArtStation: Q版 3D](https://www.artstation.com/search?query=chibi%203d%20character)

---

## ⚠️ 常见问题

### Q: 没有 3D 建模经验怎么办？

**A**: 
- Blender 学习曲线约 1-2 周入门
- Q版角色比写实角色简单得多（面数低，细节少）
- 可以用 AI 工具（Meshy）生成基础模型再修改
- 如果实在困难，可以考虑外包角色建模

### Q: 云服务器离线怎么办？

**A**: 
- P0 阶段的 Blender 工作都在本地完成
- 只有最后 UE5 测试需要云服务器
- 云服务器重启后资产可上传

### Q: Mixamo 动画不匹配 Q版 比例？

**A**: 
- Mixamo 动画基于正常人体比例
- 导入 UE5 后需要用 Retarget 调整
- Blender 中可以手动调整关键帧幅度
- 重点调整: 步幅、手臂摆动幅度、跳跃高度

### Q: NPR 效果不满意？

**A**: 
- 先做最简单的 2-tone shading（基础色+阴影色）
- 描边可以先不做（后面加）
- 参考崩坏3 截图，调整阴影硬度和颜色
- 考虑使用 UE5 Stylized Rendering 插件

---

## 🎯 立即可做的 5 件事

1. ✅ **安装 Blender 4.x** — 立即
2. ✅ **注册 Mixamo 账户** — 立即
3. ✅ **注册 Meshy.ai 试用** — 立即
4. ✅ **打开 Blender，跟着 Donut Tutorial 做** — 今天
5. ✅ **收集武僧设计参考图** — 本周

---

## 下一步

完成第一周后:
- 在云服务器恢复后上传 block-out 模型
- UE5 测试尺寸和 NPR 基础
- 根据反馈调整建模方向
