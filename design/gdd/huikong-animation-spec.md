# 武僧慧空 — 动画规格表

> **角色**: 气功小武僧 (Huikong)
> **版本**: v1.0
> **日期**: 2026-06-18
> **引擎**: UE5 5.7 (60 FPS 基准)
> **参考**: 崩坏3、原神战斗动画风格

---

## 1. 动画资产清单

### 基础移动动画

| ID | 动画名 | 来源 | 帧数 | 循环 | 优先级 | 说明 |
|---|---|---|---|---|---|---|
| M01 | Idle | 混元 #26 | 60f | ✅ | 0 | 基础待机，呼吸起伏 + 手腕微动 |
| M02 | Idle_B | 混元 #27 | 60f | ✅ | 0 | 变体待机，重心微移 |
| M03 | Walk | 混元 #23 | 24f | ✅ | 1 | 步行，武术步法，飘带随动 |
| M04 | Run | 混元 #34 | 16f | ✅ | 1 | 跑步，前倾姿势 |
| M05 | Jump_Start | 手工制作 | 8f |  | 2 | 起跳，蹬地动作 |
| M06 | Jump_Loop | 混元预设 | 12f | ✅ | 2 | 空中滞空 |
| M07 | Jump_Land | 混元 #13 | 10f | ❌ | 2 | 落地缓冲 |
| M08 | Fall | 混元预设 | 6f | ✅ | 2 | 下落 |

### 攻击动画

| ID | 动画名 | 来源 | 帧数 | 循环 | 优先级 | 说明 |
|---|---|---|---|---|---|---|
| A01 | Light1 | 手工制作 | 12f |  | 3 | 左直拳，快速前冲 |
| A02 | Light2 | 手工制作 | 12f | ❌ | 3 | 右勾拳，转体横扫 |
| A03 | Light3 | 手工制作 | 15f | ❌ | 3 | 双掌推，气功波起手 |
| A04 | Heavy | 手工制作 | 23f | ❌ | 3 | 气功波蓄力推出 |
| A05 | Skill1 | 手工制作 | 20f | ❌ | 3 | 气功波远程发射 |
| A06 | Skill2 | 手工制作 | 24f | ❌ | 3 | 升龙拳，击飞 |
| A07 | Ultimate | 手工制作 | 52f | ❌ | 4 | 如来神掌（含镜头演出） |

### 防御/受击动画

| ID | 动画名 | 来源 | 帧数 | 循环 | 优先级 | 说明 |
|---|---|---|---|---|---|---|
| D01 | Dodge | 手工制作 | 13f | ❌ | 4 | 气功瞬移闪避 |
| D02 | HitStun_Light | 混元 #8 | 11f | ❌ | 5 | 轻攻击受击 |
| D03 | HitStun_Heavy | 混元修改 | 20f | ❌ | 5 | 重攻击受击 |
| D04 | Knockdown | 混元修改 | 70f |  | 5 | 击倒 + 起身 |
| D05 | Death | 混元修改 | 30f |  | 5 | 死亡动画 |

### 动画总数: **22 个**
- 混元预设: 8 个（需修改适配）
- 手工制作: 14 个

---

## 2. Animation Blueprint 状态机

```
┌─────────────────────────────────────────────────────┐
│                  Entry State                         │
└──────────────────────┬──────────────────────────────┘
                       │
         ┌─────────────┼─────────────┐
         ▼             ▼             ▼
    ┌────────┐   ┌──────────┐   ┌──────────┐
    │Locomotion│  │ Attack   │   │  Dodge   │
    │(BlendSpace)│ │(Sequence)│   │(OneShot) │
    └────┬─────┘   └────┬─────┘   └────┬─────┘
         │              │              │
         ▼              ▼              ▼
    ┌────────┐   ┌──────────┐   ──────────┐
    │  Jump   │   │  Skill   │   │ HitStun  │
    │(State)  │   │(Sequence)│   │(OneShot) │
    └────────┘   └──────────┘   └──────────┘
                      │
                      ▼
                ┌──────────┐
                │ Ultimate │
                │(Sequence)│
                └──────────
```

### 状态转换规则

| 当前状态 | 可转换到 | 条件 | 过渡时间 |
|---|---|---|---|
| Locomotion | Attack | 攻击输入 | 0f（立即） |
| Locomotion | Dodge | 闪避输入 | 0f |
| Locomotion | Jump | 跳跃输入 + 在地面 | 0f |
| Attack | Attack | Cancel 窗口内输入 | 0f |
| Attack | Dodge | Cancel 窗口内闪避输入 | 0f |
| Attack | Locomotion | Recovery 结束 | 6f (100ms) |
| Dodge | Locomotion | Dodge 结束 | 2f (33ms) |
| Dodge | Attack | Dodge 结束后攻击输入 | 0f |
| Jump | Fall | 到达顶点 | 0f |
| Fall | Jump_Land | 接触地面 | 0f |
| Jump_Land | Locomotion | 落地动画结束 | 4f (67ms) |
| HitStun | Locomotion | 硬直结束 | 6f (100ms) |
| Knockdown | Locomotion | 起身完成 | 8f (133ms) |
| Any (非Ultimate) | Ultimate | 必杀输入 + 能量≥30 | 0f |
| Ultimate | Locomotion | 演出结束 | 12f (200ms) |

---

## 3. BlendSpace 规格

### Locomotion BlendSpace (2D)

```
              Run (600cm/s)
                ↑
                │
  Strafe_Left ←─┼─→ Strafe_Right
  (-180°)       │       (180°)
                │
              Idle (0cm/s)

X 轴: Speed (0 ~ 600 cm/s)
Y 轴: Direction (-180° ~ 180°, 0° = 前方)
```

**采样点**:

| Speed | Direction | 动画 |
|---|---|---|
| 0 | 任意 | Idle |
| 300 | 0° | Walk_Forward |
| 600 | 0° | Run_Forward |
| 300 | ±90° | Strafe_Side |
| 300 | 180° | Walk_Backward |

**混合规则**: 线性插值，速度优先

---

## 4. 动画过渡时间

| 过渡类型 | 时间 | 说明 |
|---|---|---|
| Idle ↔ Walk | 6f (100ms) | 平滑起步/停止 |
| Walk ↔ Run | 4f (67ms) | 快速加速/减速 |
| Run → Idle | 8f (133ms) | 缓冲停止（惯性） |
| Any → Attack | 0f | 立即中断（攻击优先级高） |
| Attack → Idle | Recovery 自然结束 | 不强制截断 |
| Dodge → Any | 2f (33ms) | 快速恢复 |
| Jump → Fall | 0f | 无缝衔接 |
| Fall → Land | 0f | 接触地面立即切换 |
| Land → Idle | 4f (67ms) | 落地缓冲 |
| HitStun → Idle | 6f (100ms) | 受击恢复 |

---

## 5. 蒙太奇 (AnimMontage) 规格

### Light Attack Combo Montage

```
Section 1: Light1 (0-12f)
  ├── Notify: HitBox_On (3f)
  ├── Notify: HitBox_Off (7f)
  └── Next: Section 2 (if LightAttack input in 7-9f)

Section 2: Light2 (12-24f)
  ├── Notify: HitBox_On (15f)
  ├── Notify: HitBox_Off (19f)
  └── Next: Section 3 (if LightAttack input in 19-21f)

Section 3: Light3 (24-39f)
  ├── Notify: HitBox_On (28f)
  ├── Notify: HitBox_Off (33f)
  └── End: Return to Locomotion
```

### Heavy Attack Montage

```
Section 1: Heavy (0-23f)
  ├── Notify: Charge_Start (0f) — 金色粒子开始
  ├── Notify: HitBox_On (8f)
  ├── Notify: HitBox_Off (13f)
  ├── Notify: Charge_End (13f) — 气功波发射
  └── End: Return to Locomotion
```

### Ultimate Montage

```
Section 1: Jump_Up (0-12f)
  ├── Notify: Camera_ZoomIn (0f)
  ── Next: Section 2

Section 2: Charge_Air (12-22f)
  ├── Notify: Golden_Effect_On (12f)
  └── Next: Section 3

Section 3: Slam_Down (22-32f)
  ├── Notify: Camera_TopView (22f)
  ├── Notify: HitBox_On (22f)
  ├── Notify: Screen_Shake (30f)
  └── Next: Section 4

Section 4: Land_Recovery (32-52f)
  ├── Notify: Camera_Reset (40f)
  ├── Notify: HitBox_Off (32f)
  └── End: Return to Locomotion
```

---

## 6. Animation Notify 清单

| Notify 名 | 类型 | 触发时机 | 用途 |
|---|---|---|---|
| HitBox_On | State | 攻击 Active 开始 | 激活 HitBoxComponent |
| HitBox_Off | State | 攻击 Active 结束 | 关闭 HitBoxComponent |
| Combo_Window | Branching Point | Recovery 前 50% | 检测连招输入 |
| Camera_ZoomIn | Play Sound/Effect | 必杀开始 | 触发镜头拉近 |
| Camera_Reset | Play Sound/Effect | 必杀结束 | 恢复相机 |
| Screen_Shake | Play Sound/Effect | 必杀爆发 | 触发屏幕震动 |
| Footstep_L | Play Sound | 步行左脚着地 | 脚步声 |
| Footstep_R | Play Sound | 步行右脚着地 | 脚步声 |
| Dodge_Start | Play Effect | 闪避开始 | 蓝色残影特效 |
| Dodge_End | Play Effect | 闪避结束 | 残影消散 |
| Energy_Gain | Custom | 攻击命中 | 能量+5/+12 |
| Invincible_On | State | 闪避 Active 开始 | 开启无敌帧 |
| Invincible_Off | State | 闪避 Active 结束 | 关闭无敌帧 |

---

## 7. 混元绑骨动画适配

| 混元编号 | 动作名 | 目标游戏动画 | 修改要点 |
|---|---|---|---|
| 26 | 待机-1 | Idle | 微调呼吸幅度，加入武僧手势 |
| 27 | 待机-2 | Idle_B | 重心偏移方向调整 |
| 23 | 走路-1 | Walk | 步幅缩小（腿长适配），加入武术步法 |
| 34 | 奔跑 | Run | 前倾角度增大，手臂摆动调整 |
| 1 | 回旋踢 | Light3 参考 | 改为双掌推动作，保留腿部力度感 |
| 8 | 受击 | HitStun_Light | 受击方向调整，加入武僧防御姿态 |
| 13 | 落地 | Jump_Land | 缓冲动作加强，加入气功气场 |

---

## 8. 性能预算

| 指标 | 目标值 | 说明 |
|---|---|---|
| 动画骨骼数 | ≤ 75 | 混元绑骨标准骨骼 |
| 动画文件大小 | ≤ 2MB/个 | FBX 动画数据 |
| BlendSpace 采样数 | ≤ 9 | 2D BlendSpace 3×3 |
| 同时播放动画层 | ≤ 3 | 基础 + 上半身 + 特效 |
| Animation Blueprint 复杂度 | ≤ 50 节点 | 避免过度复杂的状态机 |

---

## 9. 开发优先级

| 优先级 | 动画 | 原因 |
|---|---|---|
| **P0** | Idle, Walk, Run | 基础移动，最先需要 |
| **P0** | Light1, Light2, Light3 | 核心战斗体验 |
| **P0** | Dodge, HitStun | 防御反馈 |
| **P1** | Heavy, Skill1 | 战斗深度 |
| **P1** | Jump, Fall, Land | 3D 空间移动 |
| **P1** | Skill2 (升龙拳) | 连招变化 |
| **P2** | Ultimate | 高成本演出 |
| **P2** | Knockdown, Death | 失败反馈 |
| **P2** | Idle_B, Strafe | 细节打磨 |
