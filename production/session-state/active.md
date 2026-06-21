# Session State — 格斗萌主

*Last updated: 2026-06-21 17:00*

## Current Phase
**P0 原型 — 准备 P0 评审**

## ✅ 已完成

### 代码系统
- 48 C++ 源文件，编译通过 ✅
- PlayerCharacter + BaseEnemy (GAS 集成) ✅
- 第三人称相机系统 (ADR-008) ✅
- Lock-On 目标锁定 (ADR-009) ✅
- ComboManager 连招系统 ✅
- HitBox/HurtBox 碰撞检测 ✅
- HitStopManager + HitCameraShake ✅
- CharacterStatsComponent 数值成长 ✅
- DungeonFlow + WaveManager ✅
- EnemyAIController + BehaviorTree ✅
- HUDWidget + SignalBus ✅
- NPRMaterialUtils C++ 工具类 ✅

### 测试框架
- 101 自动化测试，100% 通过 ✅
- 覆盖：战斗/连招/碰撞/锁定/数值/信号/地牢/音频/敌人 AI ✅

### 3D 资产
- 武僧模型 FBX (40k 面) + 7 动画 ✅
- 糖果地牢 4 怪物完整管线 (52 uassets) ✅
- ABP_Huikong 动画蓝图 ✅

### 基础设施
- GPU 服务器：RTX 3090 + UE5.7.4 + Vulkan SM5 ✅
- 编译：libUnrealEditor-FirstGame.so (1.3MB) ✅
- MCP 服务器：172.25.0.86:3000 (23 工具域) ✅

## ⏳ 待 GUI 操作

| 任务 | 阻塞原因 | 预计时间 |
|------|---------|---------|
| NPR 父材质 (M_NPR_Parent) | MCP/Python 无法编辑材质图 | 30 分钟 |
| 4 怪物材质实例 | 依赖父材质 | 20 分钟 |
| 描边材质 (Inverted Hull) | 需自定义材质 | 30 分钟 |
| AnimBP 状态机 | 需蓝图编辑 | 1 小时 |
| LOD 配置 | 需 LOD 设置 | 30 分钟 |

### NPR 材质待办详情

**M_NPR_Parent 材质图结构**：
```
参数:
  - BaseColor (VectorParameter, default: 0.8, 0.6, 0.4)
  - ShadowColor (VectorParameter, default: 0.3, 0.15, 0.2)
  - RimColor (VectorParameter, default: 1.0, 0.9, 0.7)
  - ShadowThreshold (ScalarParameter, default: 0.3)

节点:
  VertexNormalWS ─┐
                   ├─ DotProduct → NdotL ─┐
  LightDir (0.5,0.3,0.8) ──┘              │
                                           ├─ Step → ShadowMask
  ShadowThreshold ─────────────────────────

  ShadowColor ────┐
                  ├─ Lerp → SurfaceColor ─
  BaseColor ──────┘                       │
                                          ├─ Add → EmissiveColor
  RimColor ───┐                           │
  0.3 ──────── Multiply → Rim ──────────
```

**4 怪物配色**：
| 怪物 | BaseColor | ShadowColor | RimColor |
|------|-----------|-------------|----------|
| CandyZombie | (0.55, 0.85, 0.60) 绿 | (0.25, 0.15, 0.30) 紫 | (0.80, 1.00, 0.70) |
| Gingerbread | (0.85, 0.55, 0.30) 棕 | (0.35, 0.12, 0.08) 深棕 | (1.00, 0.85, 0.40) 金 |
| ShadowNinja | (0.40, 0.20, 0.55) 紫 | (0.15, 0.05, 0.20) 深紫 | (0.30, 0.70, 1.00) 蓝 |
| ArmoredGum | (0.90, 0.92, 0.95) 银 | (0.15, 0.20, 0.35) 深蓝 | (1.00, 1.00, 1.00) 白 |

## 🔜 下一步

**P0 评审准备**
1. 整理 P0 完成清单
2. 准备演示关卡截图
3. 编写 P0 评审文档

**GUI 操作计划**（下次会话）
1. 通过 VNC 连接编辑器 (DISPLAY=:99)
2. 创建 M_NPR_Parent 材质
3. 创建 4 怪物材质实例
4. 配置 AnimBP 状态机
5. 设置 LOD

## 📝 关键发现

### MCP 材质编辑限制
- Python 可读取材质属性，但无法修改材质图
- MaterialEditingLibrary 通过 MCP 执行失败
- manage_asset 的材质编辑动作无响应
- **结论**：材质编辑必须通过 Editor GUI

### UE5.7 Python API 限制
- `MaterialInstanceConstantFactoryNew` 无 `Parent` 属性
- 运行时材质保存失败（文件移动错误）
- 瞬态材质 Shader 不编译
