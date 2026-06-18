# NPR 卡通渲染材质方案

> **项目**: 格斗萌主
> **风格**: 动漫风格化（参考 崩坏3 / 原神 / 绝区零）
> **引擎**: UE5 5.7
> **日期**: 2026-06-18

---

## 1. 风格目标

### 参考对标

| 游戏 | 特点 | 我们采用的 |
|---|---|---|
| 崩坏3 | 2-tone 阴影清晰、Rim Light 强 | ✅ 主要参考 |
| 原神 | 柔和过渡、环境光影响大 | 部分采用 |
| 绝区零 | 高对比、描边粗、色彩饱和 | 描边采用 |

### 核心视觉特征

```
┌─────────────────────────────────────────────┐
│  1. 2-tone 阴影（明/暗两个色阶，边界清晰）    │
│  2. Rim Light（角色边缘发光，分离背景）       │
│  3. 描边（Inverted Hull 方法，黑色轮廓）     │
│  4. 高光（小而锐利，模拟 anime 眼神光）      │
│  5. 色彩饱和（比写实高 20-30%）              │
└─────────────────────────────────────────────┘
```

---

## 2. 材质架构

### 主材质: M_AnimeToon_Character

```
┌──────────────────────────────────────────────┐
│            M_AnimeToon_Character              │
│                                               │
│  Inputs:                                      │
│  ├── BaseColor (Texture2D)     基础色贴图     │
│  ├── ShadowColor (Texture2D)   阴影色贴图     │
│  ├── Normal (Texture2D)        法线贴图       │
│  ├── Roughness (Scalar)        粗糙度 0.5     │
│  ├── RimPower (Scalar)         Rim 强度 1.5   │
│  ├── RimColor (Vector3)        Rim 颜色       │
│  ├── OutlineWidth (Scalar)     描边宽度 0.03  │
│  └── SpecularPower (Scalar)    高光锐度 32    │
│                                               │
│  Output:                                      │
│  ├── EmissiveColor   ← Rim Light + Specular  │
│  ├── BaseColor       ← 2-tone 阴影处理       │
│  └── WorldPositionOffset ← 描边膨胀          │
└──────────────────────────────────────────────┘
```

### 材质实例 hierarchy

```
M_AnimeToon_Character (父材质)
├── MI_Huikong_Body      (武僧身体)
│   ├── BaseColor: 小麦肤色
│   └── ShadowColor: 深肤色
├── MI_Huikong_Robe      (武僧僧衣)
│   ├── BaseColor: 深蓝 #1a3a52
│   └── ShadowColor: 更深蓝 #0f2030
├── MI_Huikong_Pants     (武僧裤子)
│   ├── BaseColor: 白色 #f5f5f5
│   └── ShadowColor: 浅灰 #c8c8c8
├── MI_Huikong_Ribbon    (武僧飘带)
│   ├── BaseColor: 金橙 #d4a017
│   └── ShadowColor: 深橙 #8a6a10
├── MI_Huikong_Boots     (武僧靴子)
│   ├── BaseColor: 深棕 #3d2817
│   └── ShadowColor: 近黑 #1a1008
├── MI_Sugar_Robe        (魔导师)
├── MI_Sugar_Hat         (魔导师帽子)
├── MI_Guizhan_Body      (狂战士)
└── MI_Guizhan_Armor     (狂战士铠甲)
```

---

## 3. 2-Tone 阴影实现

### 原理

```
传统 PBR:  光照 → 连续渐变的明暗
NPR 2-tone: 光照 → 阈值判断 → 只有"明"和"暗"两个色阶

         传统                    2-tone
  暗 ←————————————→ 明      暗 ←──┤明
  （平滑渐变）                    （锐利分界）
```

### UE5 Material Graph 逻辑

```
// 伪代码
float NdotL = dot(WorldNormal, LightDirection);  // 法线·光照
float ShadowThreshold = 0.3;                      // 阴影阈值

// 阴影判断
float ShadowMask = step(ShadowThreshold, NdotL);  // 0=暗, 1=明

// 颜色混合
float3 FinalColor = lerp(ShadowColor, BaseColor, ShadowMask);

// 添加光照亮度
FinalColor *= (0.5 + 0.5 * max(NdotL, 0));
```

### 阈值控制

| 参数 | 默认值 | 范围 | 效果 |
|---|---|---|---|
| ShadowThreshold | 0.3 | 0.1-0.5 | 值越大阴影面积越大 |
| ShadowSoftness | 0.05 | 0-0.2 | 边界柔化（0=完全锐利） |
| AmbientMin | 0.3 | 0.1-0.5 | 最低亮度（防止死黑） |

### 阴影方向

- **主光源方向**: 由 Directional Light 提供
- **阈值基于 NdotL**: 角色面向光的一面亮，背光的暗
- **自阴影**: 通过 ShadowColor 贴图手动绘制（AI 生成的贴图已含）

---

## 4. Rim Light 实现

### 原理

```
Rim Light = 视角方向与法线的夹角越大 → 边缘越亮

    角色正面          角色边缘
       │                │
    法线→│←视角       法线←→│视角
    夹角小              夹角大
    Rim = 0             Rim = 1（最亮）
```

### Material Graph

```
float3 ViewDir = normalize(CameraPos - WorldPos);
float3 Normal = WorldNormal;
float RimDot = 1.0 - saturate(dot(ViewDir, Normal));
float Rim = pow(RimDot, RimPower);  // RimPower 控制光带宽度

// Rim 颜色叠加到 Emissive
EmissiveColor += Rim * RimColor * RimIntensity;
```

### 参数

| 参数 | 默认值 | 说明 |
|---|---|---|
| RimPower | 1.5 | 值越大光带越窄越锐利 |
| RimIntensity | 0.8 | 整体强度 |
| RimColor | (1.0, 0.9, 0.8) | 暖白色 |

### 视觉效果

- **正面**: 几乎看不到 Rim
- **侧面**: 柔和光晕
- **边缘/轮廓**: 明亮光带（分离角色与背景）

---

## 5. 描边实现 (Inverted Hull)

### 原理

```
渲染两遍:
1. 第一遍: 正常渲染角色（正面）
2. 第二遍: 复制网格 → 法线方向外扩 → 只渲染背面 → 黑色

  ┌────      ┌────┐
  │角色│  +   │████│ (膨胀的黑色外壳)
  └────      └────┘
       =
   ┌──────┐
   │█角色█│  ← 黑色描边
   └──────┘
```

### UE5 实现方式

**方案 A: Two-Pass Material（推荐）**

```
材质设置:
- Blend Mode: Opaque
- Shading Model: Unlit
- Render CustomDepth: 启用

Pass 1 (正常渲染):
- TwoSided: No
- 正常 NPR 着色

Pass 2 (描边):
- 复制 Mesh Component
- Material: M_Outline (纯黑色)
- WorldPositionOffset: Normal * OutlineWidth
- Render Order: 在角色之后 (Translucency)
```

**方案 B: PostProcess Outline（备选）**

- 使用 CustomDepth + PostProcess Material
- Sobel 边缘检测
- 优点: 不需要复制网格
- 缺点: 描边粗细不一致（近粗远细）

### 描边参数

| 参数 | 默认值 | 范围 | 说明 |
|---|---|---|---|
| OutlineWidth | 0.03 | 0.01-0.08 | 描边粗细（世界单位） |
| OutlineColor | (0, 0, 0) | — | 描边颜色 |
| OutlineFade | 0.5 | 0-1 | 远处淡化 |

### 不同角色的描边差异

| 角色 | 描边宽度 | 说明 |
|---|---|---|
| 武僧 | 0.03 | 标准，利落 |
| 魔导师 | 0.025 | 纤细，精致 |
| 狂战士 | 0.04 | 粗犷，有力 |

---

## 6. 高光实现

### Anime 风格高光特征

```
写实高光:  大面积柔和渐变
Anime高光: 小而锐利的亮点（如眼神光、金属反光）

  写实:  ◐◐◐◐◐◐    (大范围)
  Anime:     ●       (小点)
```

### Material Graph

```
float3 HalfDir = normalize(LightDir + ViewDir);
float NdotH = saturate(dot(Normal, HalfDir));
float Specular = pow(NdotH, SpecularPower);  // 锐利高光
float SpecMask = step(0.95, Specular);       // 只保留最亮部分

EmissiveColor += SpecMask * SpecularColor * SpecularIntensity;
```

### 参数

| 参数 | 默认值 | 说明 |
|---|---|---|
| SpecularPower | 32 | 值越大高光点越小 |
| SpecularIntensity | 1.5 | 高光亮度 |
| SpecularColor | (1, 1, 1) | 白色高光 |

---

## 7. 材质创建步骤

### Step 1: 创建父材质

```
Content Browser → 右键 → Material → M_AnimeToon_Character

输入参数:
  - Texture: BaseColor, ShadowColor, Normal
  - Scalar: Roughness, RimPower, RimIntensity, OutlineWidth, SpecularPower
  - Vector3: RimColor, SpecularColor
```

### Step 2: 实现 2-tone 阴影节点

```
节点连接:
  TextureSample(BaseColor) ─┐
                              ├─ Lerp → BaseColor Output
  TextureSample(ShadowColor) ┘
  ↑
  Step(ShadowThreshold, NdotL)
```

### Step 3: 添加 Rim Light

```
CameraVector → normalize
WorldNormal  → normalize
Dot Product → 1 - saturate → Power(RimPower) → Multiply(RimColor, RimIntensity)
→ Add to EmissiveColor
```

### Step 4: 创建描边材质

```
新建 Material: M_Outline
- Shading Model: Unlit
- BaseColor: (0, 0, 0)
- WorldPositionOffset: WorldNormal * OutlineWidth

创建 Mesh Component 副本:
- MC_Outline = 复制 MC_Body
- Material: M_Outline
- Render Order: After Base
```

### Step 5: 创建材质实例

```
右键 M_AnimeToon_Character → Create Material Instance
→ MI_Huikong_Body
  - BaseColor: 武僧肤色贴图
  - ShadowColor: 武僧阴影贴图
  - RimPower: 1.5
  - OutlineWidth: 0.03
```

---

## 8. 光照设置

### Directional Light (太阳)

| 参数 | 值 | 说明 |
|---|---|---|
| Intensity | 3.0 | 比默认亮（anime 高对比） |
| Light Color | (1.0, 0.95, 0.9) | 暖白色 |
| Cast Shadows | ✅ | 需要阴影 |
| Shadow Resolution | 2048 | 清晰阴影 |

### Sky Light (环境光)

| 参数 | 值 | 说明 |
|---|---|---|
| Intensity | 0.5 | 较低（保持高对比） |
| Color | (0.6, 0.7, 0.9) | 偏蓝（天空反射） |

### PostProcess Volume

| 参数 | 值 | 说明 |
|---|---|---|
| Color Grading Saturation | 1.2 | 色彩饱和度 +20% |
| Color Grading Contrast | 1.1 | 对比度 +10% |
| Bloom Intensity | 0.3 | 轻微光晕 |
| Bloom Threshold | 2.0 | 只有很亮的地方 bloom |

---

## 9. 性能预算

| 指标 | 目标 | 说明 |
|---|---|---|
| 材质指令数 | < 200 | 保持简单 |
| 贴图采样 | 3 (BaseColor + ShadowColor + Normal) | 最小化 |
| 描边 Pass | 1 (每个角色) | 额外一个 draw call |
| 总 Draw Call (3 角色) | < 15 | 3 角色 × (1 本体 + 1 描边) + 环境 |
| 材质切换 | 0 (同一父材质) | 所有角色共享材质 |

---

## 10. 开发优先级

| 优先级 | 内容 | 说明 |
|---|---|---|
| **P0** | 2-tone 阴影基础 | 最核心的 NPR 效果 |
| **P0** | Rim Light | 角色分离感 |
| **P0** | 描边 (Inverted Hull) | 动漫轮廓感 |
| **P1** | 高光 | 细节提升 |
| **P1** | 材质实例创建 | 为每个角色/部位创建 |
| **P2** | PostProcess 调色 | 整体氛围 |
| **P2** | 特殊效果 (气功波发光) | 技能特效 |

---

## 11. 武僧配色方案 (材质参数)

### 身体 (Skin)

| 参数 | 值 |
|---|---|
| BaseColor | #c68642 (小麦色) |
| ShadowColor | #8a5a2b (深肤色) |
| RimColor | (1.0, 0.9, 0.8) |

### 僧衣 (Robe)

| 参数 | 值 |
|---|---|
| BaseColor | #1a3a52 (深蓝) |
| ShadowColor | #0f2030 (更深蓝) |
| RimColor | (0.8, 0.9, 1.0) |

### 腰带 + 飘带 (Belt/Ribbon)

| 参数 | 值 |
|---|---|
| BaseColor | #d4a017 (金橙) |
| ShadowColor | #8a6a10 (深橙) |
| RimColor | (1.0, 0.9, 0.5) |

### 裤子 (Pants)

| 参数 | 值 |
|---|---|
| BaseColor | #f5f5f5 (白色) |
| ShadowColor | #c8c8c8 (浅灰) |
| RimColor | (1.0, 1.0, 1.0) |

### 靴子 (Boots)

| 参数 | 值 |
|---|---|
| BaseColor | #3d2817 (深棕) |
| ShadowColor | #1a1008 (近黑) |
| RimColor | (0.8, 0.7, 0.5) |

---

## 12. 气功波特效材质 (M_QiBlast)

```
材质类型: Unlit + Translucent
效果: 金色半透明能量球

节点:
  Time → Sine → 脉冲缩放
  Distance(WorldPos, EmitterPos) → 径向衰减
  Fresnel → 边缘发光
  Color: 金色渐变 (#FFD700 → #FFA500)

  Final = Fresnel * Pulse * RadialFade * Color
```

| 参数 | 值 |
|---|---|
| Blend Mode | Translucent |
| BaseColor | 金色 #FFD700 |
| Opacity | 0.6-0.8 (脉冲) |
| Emissive | 金色 * 2.0 (自发光) |
