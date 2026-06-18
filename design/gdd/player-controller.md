# Player Controller (3D 版)

> **Status**: Draft (Updated for 3D, 正常比例版)
> **Last Updated**: 2026-06-18
> **Implements Pillar**: 爽快打击+糖果炸裂 (角色操控是打击感的载体)
> **Reference**: 崩坏3、战双帕弥什、原神

## Summary

玩家控制器管理角色在3D空间中的移动、跳跃、闪避、锁定和动作状态机。它消费输入系统的事件，驱动角色的物理表现和动画，并向战斗系统提供角色状态查询接口。核心目标是让角色移动"跟手"、闪避"灵敏"、锁定切换"流畅"。

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `Input System`, `Camera System`

## Overview

Player Controller 是角色的"身体系统"——它决定角色如何在3D空间中响应玩家输入。移动速度、跳跃弧线、闪避距离、锁定跟随、摄像机交互……这些参数共同定义了角色的"手感"。3D动作游戏的角色必须感觉"灵活"且"有分量"，任何滑动或延迟都会破坏战斗体验。

## Player Fantasy

角色像手指的延伸——想往哪走就往哪走，想闪避就闪避，想锁定就锁定。角色的身材不应该意味着慢吞吞，反而应该是"灵活机动"的感觉。着地瞬间有弹性，起跳瞬间有爆发，闪避瞬间有残影，锁定切换时镜头流畅旋转。

## Camera Modes (镜头模式)

### 自由模式 (Free Look)
- **触发条件**: 非战斗状态、无锁定目标
- **镜头位置**: 第三人称越肩，距离角色2.5m，高度在角色头顶上方0.5m
- **控制方式**: 右摇杆/鼠标右键拖动可自由旋转
- **角色朝向**: 相对于镜头方向移动（W=向前，S=向后，A=左，D=右）
- **自动回正**: 停止旋转3秒后，镜头缓慢回到角色背后

### 战斗模式 (Combat Lock)
- **触发条件**: 锁定敌人时（按Tab/手柄R或自动锁定）
- **镜头位置**: 拉近至2.0m，锁定目标位于屏幕中心偏上1/3
- **角色朝向**: 始终面向锁定目标（除非在攻击收招）
- **移动方式**: 相对锁定目标的环绕移动（W=靠近目标，S=远离，A/D=环绕）
- **切换目标**: 按Tab/手柄R在视野范围内切换锁定目标
- **镜头跟随**: 目标移动时镜头平滑跟随，保持目标在屏幕中心

### 闪避模式 (Dodge)
- **触发条件**: 闪避中，特别是极限闪避（敌人攻击即将命中时）
- **镜头效果**: 子弹时间（0.3x时间膨胀）+ 镜头轻微拉近
- **持续时间**: 0.5秒后恢复正常速度

### 必杀演出 (Ultimate)
- **触发条件**: 释放必杀技
- **镜头效果**: 镜头拉近至角色特写，背景虚化，持续1-2秒后拉回战斗视角

## Detailed Design

### Core Rules

1. **移动**: 3D全向移动（WASD/左摇杆），带加速/减速曲线（崩坏3式）
2. **跳跃**: 单段跳，空中可攻击一次（着地重置）
3. **闪避**: 空格键/手柄B，3D空间全向闪避（无敌帧）
4. **冲刺**: Shift键/手柄LB，快速直线位移（消耗体力）
5. **锁定**: Tab键/手柄R，锁定最近敌人（按住锁定或切换锁定可配置）
6. **面向**: 战斗模式下面向锁定目标，自由模式下面向移动方向
7. **地面检测**: 使用 UE5 CharacterMovement 的 IsMovingOnGround()，配合土狼时间

### States and Transitions

```
              ┌──────────┐
         ┌────│  Idle    │◄────────────┐
         │    └────┬─────┘             │
         │         │move               │stop/land
         │    ┌────▼─────┐             │
         │    │  Run     │─────────────┤
         │    └────┬─────┘             │
         │         │jump               │
         │    ┌────▼─────┐             │
         │    │  Jump    │─────────────┤
         │    └────┬─────┘             │
         │         │fall               │
         │    ┌────▼─────┐             │
         │    │  Fall    │─────────────┤
         │    └────┬─────┘             │
         │         │attack             │timeout/hit
         │    ┌────▼─────┐             │
         │    │ Attack   │─────────────┘
         │    └────┬─────┘
         │         │dodge
         │    ┌────▼─────┐
         │    │  Dodge   │─────────────► Idle
         │    └──────────┘
         │         │hit
         │    ┌────▼─────┐
         └───►│ HitStun  │─────────────► Idle
              └──────────┘
              │sprint
         ┌────▼─────┐
         │  Sprint  │─────────────► Run (on stop)
         └──────────┘
```

| State | Entry | Exit | Movement | Can Attack | Can Dodge | Can Lock |
|---|---|---|---|---|---|---|
| Idle | 着地+无输入 | 移动/跳跃/攻击/闪避/冲刺/被击 | 无 | Yes | Yes | Yes |
| Run | 着地+方向输入 | 停止/跳跃/攻击/闪避/冲刺/被击 | 全速 | Yes | Yes | Yes |
| Jump | 离地(跳跃) | 开始下落/空中攻击 | 空中控制率35% | Yes(空中攻) | No | Yes |
| Fall | 下落阶段 | 着地 | 空中控制率35% | Yes(空中攻) | No | Yes |
| Attack | 攻击输入 | 动作结束/被击 | 攻击位移 | 连招可 | Yes(中断) | Auto |
| Dodge | 闪避输入 | 闪避结束/着地 | 闪避速度 | No | No | Auto |
| HitStun | 被击中 | 硬直结束 | 击退 | No | No | No |
| Sprint | Shift/双击方向 | 停止/攻击/闪避/被击/体力空 | 冲刺速度 | No | Yes | No |

### Movement Parameters (UE5 units: cm/s)

| Parameter | Value | Notes |
|---|---|---|
| 地面移动速度 | 600 cm/s | 崩坏3参考（约6m/s） |
| 冲刺速度 | 1200 cm/s | 2x普通速度 |
| 空中控制率 | 35% | 比2D更低（3D空中更难精确控制） |
| 跳跃初速度 | 800 cm/s | 跳跃高度约2m |
| 重力加速度 | 2040 cm/s² | 跳跃总时长约0.8s |
| 跳跃截断系数 | 0.5 | 松开跳跃键时上升速度截断 |
| 土狼时间 | 100ms | 离开平台后仍可跳跃的窗口 |
| 跳跃缓冲 | 150ms | 着地前提前按跳跃也有效 |
| 闪避速度 | 1400 cm/s | 快速位移 |
| 闪避距离 | 300 cm | 闪避总位移（约3m） |
| 闪避时长 | 220ms | 闪避帧数 |
| 闪避无敌帧 | 前150ms | 约7帧无敌 |
| 体力消耗(闪避) | 15 | 最大体力100 |
| 体力消耗(冲刺)/秒 | 20 | 持续消耗 |
| 体力恢复/秒 | 30 | 非动作时恢复 |
| 击退初始速度 | 400 cm/s | 被击中后退 |
| 击退摩擦 | 1200 cm/s² | 快速减速 |
| 加速度 | 2400 cm/s² | 达到最大速度的时间 ≈ 0.25s |
| 减速度 | 3000 cm/s² | 停止时间 ≈ 0.2s |

### Lock-On 系统

```
锁定规则:
- 范围: 以角色为圆心, 2000cm 半径球体
- FOV: 以相机前方为轴, ±60° 锥形
- 优先级: HP% 最低 > 距离最近 > 当前锁定
- 切换延迟: 200ms (防止快速切换抖动)
- 失去锁定: 目标离开FOV超过3秒 / 目标死亡 / 目标离开竞技场
```

| Lock State | Camera Behavior | Movement | Attack |
|---|---|---|---|
| 无锁定 | 自由越肩 | 相对镜头 | 朝向移动方向 |
| 锁定中 | 目标居中 | 环绕目标 | 始终面向目标 |
| 锁定切换中 | 镜头旋转 | 保持锁定 | 新目标自动锁定 |

## Interactions with Other Systems

| System | Interaction | Interface |
|---|---|---|
| Input System | 消费输入事件 | GetMovementInput(), IsActionPressed() |
| Camera System | 驱动相机模式切换 | SetCameraMode(EFree/Lock/Dodge/Ultimate) |
| LockOn System | 锁定目标查询 | GetCurrentTarget(), SwitchTarget() |
| Combat System | 提供角色状态 | GetState(), IsOnGround(), GetFacing() |
| Combo System | 攻击状态驱动连招 | Broadcast: OnStateChanged |
| Hit Detection | 被击中切换HitStun | EnterHitStun(KnockbackVector) |
| Animation | 状态驱动动画 | BlendSpace/AnimNotify |
| Stamina System | 闪避/冲刺消耗体力 | ConsumeStamina(amount) |

## Formulas

### 3D Movement (加速度模型)

```cpp
// 目标速度方向（相对镜头）
FVector TargetVelocity = CameraForward * MoveInput.Y + CameraRight * MoveInput.X;
TargetVelocity.Normalize();
TargetVelocity *= MaxWalkSpeed;

// 平滑过渡到目标速度
CurrentVelocity = FMath::VInterpTo(
    CurrentVelocity, TargetVelocity, DeltaTime, 
    (TargetVelocity.Size() > 0) ? AccelerationRate : DecelerationRate
);
```

### Variable Jump Height

```cpp
if (JumpReleased && VerticalVelocity > 0) {
    VerticalVelocity *= JumpCutoffFactor;  // 0.5
}
```

### Knockback Decay (3D)

```cpp
// 击退方向由攻击方向决定
KnockbackVelocity = FMath::VInterpTo(KnockbackVelocity, FVector::ZeroVector, DeltaTime, KnockbackDecayRate);
```

### Lock-On 移动 (相对目标)

```cpp
// 以锁定目标为原点，计算相对移动方向
FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();
FVector Right = FVector::CrossProduct(FVector::UpVector, ToTarget);

// WASD输入映射到相对方向
FVector MoveDirection = ToTarget * Input.Y + Right * Input.X;
```

## 3D 特殊 Edge Cases

| Scenario | Expected Behavior | Rationale |
|---|---|---|
| 空中攻击着地 | 攻击状态立即结束→Idle | 着地中断空中动作 |
| 闪避中被击中 | 无敌帧内不受击，外则受击 | 无敌帧150ms |
| 锁定目标死亡 | 自动切换到最近目标 / 释放锁定 | 流畅体验 |
| 多个敌人重叠 | 优先锁定HP最低的 | 战术合理性 |
| 镜头穿墙 | SpringArm自动拉近 | 避免视觉阻塞 |
| 闪避到竞技场边缘 | 被隐形墙阻挡 | 保持战斗密度 |
| 锁定模式下背对镜头攻击 | 角色绕锁定目标转身攻击 | 保持面向目标 |
| 空中闪避 | 不允许（只有地面闪避） | 防止无限浮空 |
| 冲刺撞墙 | 冲刺停止，不反弹 | 避免镜头抖动 |

## Visual/Audio Requirements (3D)

| Event | Visual Feedback | Audio Feedback | Priority |
|---|---|---|---|
| 着地 | 角色挤压+小灰尘粒子+镜头轻微震动 | 着地轻响 | High |
| 起跳 | 角色拉伸+弹跳 | 起跳音效 | Medium |
| 闪避 | 半透明残影(5帧)+运动模糊 | 闪避风声 | High |
| 极限闪避 | 子弹时间+镜头拉近+角色表情特写 | 慢动作音效 | Critical |
| 锁定切换 | 锁定标记平滑旋转 | 切换提示音 | Medium |
| 冲刺 | 拖尾粒子+速度线 | 冲刺风声 | Medium |
| 被击中 | 红色闪烁+击退形变+镜头震动 | 受击音效 | High |

## Tuning Knobs (3D)

| Parameter | Current | Safe Range | Increase Effect | Decrease Effect |
|---|---|---|---|---|
| move_speed | 600 | 400-800 | 更快，难精确站位 | 更慢，更精确但不够爽 |
| dodge_distance | 300cm | 200-500cm | 闪避更远 | 闪避更近 |
| dodge_invulnerable | 150ms | 100-250ms | 更宽容 | 更严苛 |
| lock_range | 2000cm | 1500-3000cm | 可锁更远目标 | 只能锁近处 |
| camera_distance | 250cm | 200-400cm | 视野更广 | 更贴近角色 |
| air_control | 0.35 | 0.2-0.6 | 空中更灵活 | 空中更僵硬 |

## Feel Acceptance Criteria (3D)

- [ ] WASD输入1帧内角色开始移动，松开1帧内开始减速
- [ ] 短按跳跃和长按跳跃有明显高度差异（30% vs 100%）
- [ ] 闪避在150ms内完全无敌，测试者可用闪避可靠躲避Boss攻击
- [ ] 锁定切换在200ms内完成，镜头平滑过渡
- [ ] 极限闪避触发子弹时间+镜头拉近，持续0.5秒
- [ ] 着地时有可见的挤压动画和镜头微震
- [ ] 自由模式下3秒无操作镜头缓慢回正
- [ ] 冲刺撞墙时不反弹，平滑停止
- [ ] 锁定模式下角色始终面向锁定目标

## Dependencies

| System | Direction | Nature |
|---|---|---|
| Input System | ← depends on | 消费输入事件 |
| Camera System | ↔ bidirectional | 驱动相机模式 / 接收相机状态 |
| LockOn System | → depended by | 提供角色位置 / 接收目标切换 |
| Combat System | → depended by | 查询角色状态 |
| Hit Detection | ↔ bidirectional | 被击→进入硬直 / 硬直→不可攻击 |
| Animation System | → depended by | 状态驱动动画 |
| Stamina System | ← depends on | 闪避/冲刺消耗体力 |
