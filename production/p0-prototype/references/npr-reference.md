# NPR 卡通渲染参考

> **用途**: UE5 中实现糖果炸裂风 Q版角色渲染
> **参考游戏**: 崩坏3, 原神, 绝区零, 罪恶装备

## 目标效果

```
崩坏3 风格:
├── 2-tone shading (亮部+暗部, 硬边过渡)
├── 描边 (黑色, 略粗)
├── Rim Light (边缘光)
├── 鲜艳主色 + 深暗阴影色
└── 技能特效: 高饱和, 粒子多

糖果炸裂风:
├── 更鲜艳的配色 (比崩坏3更甜)
├── 阴影色偏冷 (暗色地牢对比)
├── 高光更亮 (糖果质感)
├── 特效粒子: 彩色, 炸裂感
└── 角色变形: 挤压拉伸
```

## UE5 Toon Shader 基础节点

```
Material Graph (简化):

[Base Color] ──────────────────┐
                                │
[Light Vector] → [Dot Product] → [Step] → [Lerp(Base, Shadow)]
                                │
[Rim Light] ← [Fresnel] ← [View Vector]
                                │
[Outline] ← [Second Material, 反向法线, 黑色]

最终 = ToonShaded + RimLight
描边 = 单独 Pass
```

### 关键参数

| 参数 | 默认值 | 范围 | 作用 |
|---|---|---|---|
| Shadow Threshold | 0.5 | 0-1 | 亮暗过渡位置 |
| Shadow Softness | 0.05 | 0-0.3 | 过渡软硬 (Q版用硬边) |
| Shadow Color | #3A3A5A | 任意 | 阴影色 (通常偏冷) |
| Rim Power | 3.0 | 1-10 | Rim Light 强度分布 |
| Rim Color | #FFFFFF | 任意 | Rim Light 颜色 |
| Outline Width | 0.02 | 0.005-0.05 | 描边宽度 |
| Outline Color | #000000 | 任意 | 描边颜色 |

## 描边方法对比

| 方法 | 优点 | 缺点 | 推荐 |
|---|---|---|---|
| **Inverted Hull** | 简单, 性能好 | 复杂模型角落效果差 | ✅ P0 首选 |
| PostProcess | 任意模型统一描边 | 性能开销, 细节丢失 | P2 后期 |
| Geometry Shader | 精准控制 | UE5 支持有限 | 不推荐 |

### Inverted Hull 实现

```
1. 复制角色 Mesh
2. 复制的 Mesh 法线全部反向 (Flip Normals)
3. 材质: Unlit + 黑色 (或主色暗版)
4. 材质中 Vertex Shader 沿法线挤出 (0.5-2cm)
5. 设置复制 Mesh Render After 主 Mesh
```

## 阴影色设计 (糖果炸裂风)

### 各职业阴影色

| 职业 | 主色 | 阴影色 | 说明 |
|---|---|---|---|
| 武僧 | #6699FF | #2A3A7A | 蓝色偏紫暗 |
| 魔导师 | #FF6B9D | #7A2A4A | 粉色偏紫暗 |
| 狂战士 | #9B59B6 | #4A2A5A | 紫色更深 |

### 皮肤阴影

```
基础肤色: #FFD4B3 (暖色)
阴影肤色: #D4A583 (偏冷, 带紫)

Q版皮肤阴影特点:
- 阴影色明显(对比强)
- 阴影偏冷色(避免脏)
- 脸颊可加淡红(Rim Light)
```

## Rim Light 设置

```
目的: 让角色从暗背景中突出

参数:
- Power: 3-5 (较集中)
- Color: 与主色同系, 但更亮更饱和
- Intensity: 0.5-1.0

武僧 Rim Light: #88BBFF (亮蓝)
魔导师 Rim Light: #FF99BB (亮粉)
狂战士 Rim Light: #BB88DD (亮紫)
```

## UE5 材质节点图 (简化版)

```
ToonShading:
├── 输入: BaseColor, ShadowColor, LightVector, ViewVector
├── DotNL = Dot(Normal, LightVector)
├── ShadowMask = Step(ShadowThreshold - ShadowSoftness, DotNL)
├── Diffuse = Lerp(ShadowColor, BaseColor, ShadowMask)
├── Fresnel = Power(Base=1, Exponent=RimPower)
├── Rim = Fresnel * RimColor * RimIntensity
└── 输出 = Diffuse + Rim
```

## PostProcess 建议

```
后处理链:
├── Bloom (强度 0.5) — 特效发光
├── Color Grading
│   ├── Saturation +10% (糖果鲜艳)
│   ├── Contrast +5% (打击感)
│   └── Gamma 1.0
└── Ambient Occlusion (强度 0.3) — 角色落地感
```

## 测试流程

```
Week 1:
├─ 创建测试场景 (暗色地牢环境)
├─ 导入简单立方体测试 NPR 基础
├─ 调整 Shadow Threshold/Softness
└─ 确定描边方法

Week 2:
├─ 导入武僧模型
├─ 创建 M_Huikong_Toon 材质
├─ 测试 Base Color + Shadow Color
└─ 测试 Rim Light

Week 3:
├─ 完整材质调试
├─ 描边调整
├─ 动态测试 (动画时效果稳定)
└─ 性能测试 (帧率 > 60fps)
```

## 性能预算

| 元素 | Budget |
|---|---|
| Toon Shader | < 0.5ms |
| 描边 Pass | < 0.3ms |
| 角色渲染总开销 | < 1.0ms |
| 目标帧率 | 60fps (16.6ms) |

## 参考截图收集清单

在本地游戏或网上收集以下截图:

- [ ] 崩坏3 — 角色材质 (战斗中)
- [ ] 崩坏3 — 必杀演出 (特写)
- [ ] 崩坏3 — 描边效果 (近距离)
- [ ] 原神 — 角色 NPR (白天/夜晚)
- [ ] 原神 — 特效 (元素反应)
- [ ] 绝区零 — 战斗特效
- [ ] 罪恶装备 -Strive- — 2.5D 渲染
- [ ] 龙珠Z Kakarot — 气功波特效
