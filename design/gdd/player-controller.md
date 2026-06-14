# Player Controller

> **Status**: Draft
> **Author**: game-designer + gameplay-programmer
> **Last Updated**: 2026-06-15
> **Last Verified**: —
> **Implements Pillar**: 爽快打击+Q版反差 (角色操控是打击感的载体)

## Summary

玩家控制器管理角色的移动、跳跃、面向切换和动作状态机。它消费输入系统的事件，驱动角色的物理表现和动画，并向战斗系统提供角色状态查询接口。核心目标是让角色移动"跟手"，跳跃"有弹性"，状态切换"不卡顿"。

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `Input System`

## Overview

Player Controller 是角色的"身体系统"——它不决定角色做什么（那是玩家输入决定的），但决定角色怎么做。移动速度、跳跃弧线、重力感、地面摩擦、空中控制……这些参数共同定义了角色的"手感"。横版格斗游戏的角色必须感觉"贴地"且"灵敏"，任何延迟或滑动都会破坏连招体验。

## Player Fantasy

角色像手指的延伸——想左走就左走，想跳就跳，想打就打。Q版角色的短腿不应该意味着慢吞吞，反而应该是"小身体大机动"的反差感。着地瞬间有弹性，起跳瞬间有爆发，闪避瞬间有残影。

## Detailed Design

### Core Rules

1. **移动**: 左右移动基于输入轴值，即时加速到最大速度（无加速曲线），松手即时停止（无减速曲线）
2. **跳跃**: 单段跳，按下瞬间给予向上初速度，重力持续作用；松开跳跃键时如果仍在上升则减少上升速度（可变跳跃高度）
3. **面向**: 攻击/移动时自动面向输入方向；不移动时保持最后面向
4. **地面检测**: 使用 Godot CharacterBody2D 的 is_on_floor()，配合土狼时间（Coyote Time）
5. **动作锁**: 攻击/闪避期间限制移动速度（攻击推力除外），不限制面向切换

### States and Transitions

```
         ┌──────────┐
    ┌────│  Idle    │◄────┐
    │    └────┬─────┘     │
    │         │move       │stop/land
    │    ┌────▼─────┐     │
    │    │  Run     │─────┤
    │    └────┬─────┘     │
    │         │jump       │
    │    ┌────▼─────┐     │
    │    │  Jump    │─────┤
    │    └────┬─────┘     │
    │         │fall       │
    │    ┌────▼─────┐     │
    │    │  Fall    │─────┤
    │    └────┬─────┘     │
    │         │attack     │
    │    ┌────▼─────┐     │
    │    │ Attack   │─────┘ timeout/hit
    │    └────┬─────┘
    │         │dodge
    │    ┌────▼─────┐
    │    │  Dodge   │─────► Idle
    │    └──────────┘
    │         │hit
    │    ┌────▼─────┐
    └───►│ HitStun  │─────► Idle
         └──────────┘
```

| State | Entry Condition | Exit Condition | Movement | Can Attack | Can Dodge |
|-------|----------------|----------------|----------|-----------|-----------|
| Idle | 着地+无输入 | 移动/跳跃/攻击/闪避/被击 | 无 | Yes | Yes |
| Run | 着地+方向输入 | 停止/跳跃/攻击/闪避/被击 | 全速 | Yes | Yes |
| Jump | 离地(跳跃) | 开始下落 | 空中控制率70% | Yes(空中攻) | No |
| Fall | 下落阶段 | 着地 | 空中控制率70% | Yes(空中攻) | No |
| Attack | 攻击输入 | 动作计时结束 | 攻击推力 | No(连招除外) | Yes(中断) |
| Dodge | 闪避输入 | 闪避距离完成 | 闪避速度 | No | No |
| HitStun | 被击中 | 硬直结束 | 击退减速 | No | No |

### Movement Parameters

| Parameter | Value | Notes |
|-----------|-------|-------|
| 地面移动速度 | 300 px/s | 即时到达，无加速 |
| 空中控制率 | 70% | 空中可变向，但不如地面灵敏 |
| 跳跃初速度 | -500 px/s | 负方向=向上 |
| 重力加速度 | 980 px/s² | 接近真实感 |
| 跳跃截断系数 | 0.4 | 松开跳跃键时上升速度乘以此值 |
| 土狼时间 | 80ms | 离开平台后仍可跳跃的窗口 |
| 跳跃缓冲 | 150ms | 着地前提前按跳跃也有效 |
| 攻击推力 | 50 px/s | 攻击时轻微前冲 |
| 闪避速度 | 500 px/s | 快速位移 |
| 闪避距离 | 120 px | 闪避总位移 |
| 闪避时长 | 200ms | 闪避帧数 |
| 击退初始速度 | 200 px/s | 被击中后退 |
| 击退摩擦 | 600 px/s² | 快速减速 |

### Interactions with Other Systems

| System | Interaction | Interface |
|--------|-------------|-----------|
| Input System | 消费输入事件 | get_vector(), is_action_just_pressed() |
| Combat System | 提供角色状态查询 | get_state(), is_on_floor(), get_facing() |
| Combo System | 攻击状态驱动连招 | signal: state_changed(state) |
| Hit Detection | 被击中时切换HitStun | method: enter_hit_stun(knockback) |
| Animation | 状态驱动动画切换 | signal: state_changed(state) |
| Camera | 闪避/跳跃时镜头微调 | signal: screen_offset_requested(offset) |

## Formulas

### Variable Jump Height

```
if jump_released and velocity.y < 0:
    velocity.y *= jump_cutoff_factor
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| velocity.y | float | -500 to 0 | physics | 当前垂直速度 |
| jump_cutoff_factor | float | 0.3-0.6 | config | 松开跳跃键后的速度截断系数 |

**Expected output range**: 短按跳跃≈40px高度，长按跳跃≈128px高度
**Edge case**: 在上升速度接近0时松开跳跃键，截断效果可忽略（自然到达顶点）

### Knockback Decay

```
velocity.x = move_toward(velocity.x, 0, knockback_friction * delta)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| velocity.x | float | -500 to 500 | physics | 当前水平速度 |
| knockback_friction | float | 300-800 | config | 击退减速速率(px/s²) |
| delta | float | 0.016 | engine | 帧间隔 |

**Expected output range**: 击退从200px/s在~0.33秒内减速到0

### Dodge Movement

```
dodge_position = start_position + facing * dodge_distance * (elapsed / dodge_duration)
```

**Expected output range**: 120px位移，200ms内完成

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|----------|------------------|-----------|
| 空中攻击着地 | 攻击状态立即结束→Idle | 着地中断空中动作 |
| 闪避中被击中 | 闪避无敌帧内不受击，无敌帧外受击 | 闪避有8帧无敌窗口 |
| 连招最后一击仍在空中 | 着地后才可执行下一动作 | 空中不能无限连招 |
| 同时按左+右 | 不移动（轴值抵消为0） | 避免抖动 |
| 跳跃缓冲+土狼时间同时触发 | 土狼时间优先（刚离开平台），缓冲在着地后触发 | 防止双重跳 |
| 攻击中按反方向 | 不改变面向，但攻击推力方向不变 | 攻击期间面向锁定 |

## Dependencies

| System | Direction | Nature of Dependency |
|--------|-----------|---------------------|
| Input System | This depends on Input | 消费输入事件 |
| Combat System | Combat depends on this | 查询角色状态 |
| Combo System | Combo depends on this | 攻击状态驱动连招 |
| Hit Detection | Bidirectional | 被击→进入硬直，硬直→不可攻击 |
| Animation System | Animation depends on this | 状态驱动动画 |

## Tuning Knobs

| Parameter | Current Value | Safe Range | Effect of Increase | Effect of Decrease |
|-----------|--------------|------------|-------------------|-------------------|
| move_speed | 300 px/s | 200-500 | 更快移动，更难精确站位 | 更慢，更精确但不够爽 |
| jump_velocity | -500 px/s | -300 to -700 | 跳得更高更远 | 跳得更低更近 |
| gravity | 980 px/s² | 600-1500 | 更快下落，跳跃更"重" | 更慢下落，更"飘" |
| jump_cutoff | 0.4 | 0.2-0.7 | 短跳更低(更精确) | 短跳更高(更宽容) |
| coyote_time | 80ms | 0-150ms | 离开平台后跳跃窗口更宽 | 必须精确在平台上跳 |
| air_control | 0.7 | 0.3-1.0 | 空中更灵活 | 空中更僵硬 |
| dodge_speed | 500 px/s | 300-700 | 闪避更快更远 | 闪避更慢更短 |
| attack_push | 50 px/s | 0-150 | 攻击前冲更大 | 攻击原地不动 |

## Visual/Audio Requirements

| Event | Visual Feedback | Audio Feedback | Priority |
|-------|----------------|---------------|----------|
| 着地 | 角色挤压+小灰尘粒子 | 着地轻响 | Medium |
| 起跳 | 角色拉伸+小弹跳 | 起跳音效 | Medium |
| 闪避 | 半透明残影(3帧) | 闪避风声 | High |
| 被击中 | 红色闪烁+击退形变 | 受击音效 | High |
| 转向 | 短暂挤压(2帧) | 无 | Low |

## Game Feel

### Feel Reference
应像《洛克人X》的移动手感——即时响应+精确控制+弹性跳跃。Q版角色应该比写实角色更"弹"，着地/起跳都有明显的挤压拉伸。**不应像**《恶魔城》早期的惯性滑行——那种"刹不住车"的感觉在连招格斗中是灾难。

### Input Responsiveness

| Action | Max Input-to-Response Latency (ms) | Frame Budget (at 60fps) | Notes |
|--------|-----------------------------------|------------------------|-------|
| 移动 | 16ms | 1帧 | 即时，无加速曲线 |
| 跳跃 | 33ms | 2帧 | 含土狼时间补偿 |
| 攻击启动 | 33ms | 2帧 | 从输入到动画首帧 |
| 闪避 | 33ms | 2帧 | 逃脱动作必须即时 |
| 状态切换 | 50ms | 3帧 | 收招到下一动作 |

### Animation Feel Targets

| Animation | Startup Frames | Active Frames | Recovery Frames | Feel Goal | Notes |
|-----------|---------------|--------------|----------------|-----------|-------|
| Idle→Run | 2 | — | — | 瞬间起步 | 无加速过渡 |
| Run→Idle | 2 | — | — | 瞬间停止 | 无减速滑行 |
| Jump起跳 | 1 | — | — | 爆发离地 | 挤压→拉伸 |
| Jump着地 | 2 | — | — | 弹性着陆 | 拉伸→挤压→恢复 |
| Dodge | 1 | 8 | 3 | 快进快出 | 无敌帧在Active期间 |
| HitStun | — | 8-16 | 4 | 沉重受击 | 视伤害等级 |

### Impact Moments

| Impact Type | Duration (ms) | Effect Description | Configurable? |
|-------------|--------------|-------------------|---------------|
| 着地挤压 | 67ms | 4帧挤压(Y:1.2 X:0.8)→恢复 | Yes |
| 起跳拉伸 | 50ms | 3帧拉伸(Y:0.7 X:1.3)→恢复 | Yes |
| 闪避残影 | 133ms | 3个半透明残影，间隔2帧 | Yes |
| 受击变形 | 100ms | 横向压缩0.6x+弹回 | Yes |

### Weight and Responsiveness Profile

- **Weight**: 轻盈但有力——Q版角色的"轻"不代表"飘"，着地有分量
- **Player control**: 极高——所有动作可在2帧内取消切换，除了必杀收招
- **Snap quality**: 极致二元——移动/停止/转向全部即时，无渐变
- **Acceleration model**: 纯arcade——即时加速即时减速，零惯性零漂移
- **Failure texture**: 被击中时有明确的硬直视觉+操作剥夺感，玩家知道"我被打到了"

### Feel Acceptance Criteria

- [ ] 按下方向键1帧内角色开始移动，松开1帧内完全停止
- [ ] 短按跳跃和长按跳跃有明显高度差异
- [ ] 土狼时间窗口内离平台后跳跃100%成功
- [ ] 着地时有可见的挤压动画（无测试者描述角色为"飘"）
- [ ] 闪避在8帧内完全无敌，测试者可用闪避可靠躲避攻击

## UI Requirements

| Information | Display Location | Update Frequency | Condition |
|-------------|-----------------|-----------------|-----------|
| 角色状态(调试) | 开发者HUD | 每帧 | Debug模式 |
| 闪避冷却 | HUD技能栏 | 每帧 | 闪避使用后 |

## Cross-References

| This Document References | Target GDD | Specific Element Referenced | Nature |
|--------------------------|-----------|----------------------------|--------|
| 输入事件消费方式 | `design/gdd/input-system.md` | get_vector(), is_action_just_pressed() | Data dependency |
| 攻击状态驱动连招 | `design/gdd/combo-system.md` | combo_step计数 | State trigger |
| 被击中进入硬直 | `design/gdd/combat-system.md` | hit_stun_duration | Rule dependency |
| 着地/闪避/受击特效 | `design/art/art-bible.md` | 变形规则 | Rule dependency |

## Acceptance Criteria

- [ ] 所有7个状态间转换正确，无非法状态可达
- [ ] 土狼时间80ms窗口内跳跃100%成功
- [ ] 跳跃缓冲150ms内着地后自动触发跳跃
- [ ] 可变跳跃高度：短按约为长按高度的30%
- [ ] 攻击推力50px/s，不叠加（每次攻击重置）
- [ ] 闪避200ms内完成120px位移，8帧无敌窗口
- [ ] Performance: _physics_process耗时<1ms
- [ ] No hardcoded values in implementation

## Open Questions

| Question | Owner | Deadline | Resolution |
|----------|-------|----------|-----------|
| 二段跳是否作为技能解锁？ | game-designer | Skill Tree GDD时 | 倾向VS阶段解锁 |
| 墙壁跳？ | game-designer | GDD审批时 | MVP不做，VS阶段考虑 |
| 冲刺(dash)vs闪避(dodge)？ | game-designer | Combat System GDD时 | 合并为闪避，含无敌帧 |
