# Hit Detection & Feedback

> **Status**: Draft
> **Author**: gameplay-programmer
> **Last Updated**: 2026-06-15
> **Last Verified**: —
> **Implements Pillar**: 爽快打击+Q版反差 (命中反馈是打击感的灵魂)

## Summary

命中检测与反馈系统负责判定攻击是否命中、计算伤害数字、触发打击反馈（命中停顿/屏幕震动/伤害数字/形变），以及管理伤害类型视觉编码。它是"小身体大冲击"反差爽的直接实现者——战斗系统决定打不打得到，本系统决定打到后有多爽。

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `Player Controller`

## Overview

当攻击判定框（hitbox）与目标受击框（hurtbox）重叠时，本系统接管后续一切：确认命中、计算伤害（含暴击）、触发命中停顿、屏幕震动、伤害数字弹出、目标形变。这些反馈层叠在一起，让Q版角色的一拳感觉像砸了一座山。

## Player Fantasy

每次命中都要有"砰"的感觉——画面冻结一瞬让玩家"看到"了击中，屏幕晃一下让玩家"感觉到"了冲击，数字弹出来让玩家"确认"了伤害。轻攻轻快如蜂，重攻沉如有锤，必杀如核爆。暴击要让玩家不自觉地喊出声。

## Detailed Design

### Core Rules

1. **碰撞检测**: 使用 Godot Area2D + CollisionShape2D，hitbox 在 layer 3，hurtbox 在 layer 2
2. **命中确认**: hitbox overlap hurtbox 时触发，每帧只检测一次
3. **多目标**: 单次攻击可命中多个目标（AOE/穿透），但同一目标同一攻击只命中一次
4. **无敌帧**: 受击后有短暂无敌时间，防止同一攻击多次判定
5. **伤害计算**: 基础伤害 × 连招系数 × 暴击系数，结果取整
6. **反馈触发顺序**: 命中停顿→形变→伤害数字→屏幕震动（严格顺序，不可并行）

### Hit Detection Flow

```
Hitbox overlaps Hurtbox
    │
    ├─ Check: Is target in i-frames? → YES: Ignore
    │
    ├─ Check: Has this attack already hit this target? → YES: Ignore
    │
    ├─ Confirm hit
    │   ├─ Calculate damage
    │   ├─ Apply hit-stop (freeze game time)
    │   ├─ Apply target squash-stretch
    │   ├─ Spawn damage number
    │   ├─ Apply screen shake
    │   ├─ Notify attacker (combo step callback)
    │   └─ Grant target i-frames
    │
    └─ Disable hitbox (single-hit per activation)
```

### States and Transitions

| State | Entry Condition | Exit Condition | Behavior |
|-------|----------------|----------------|----------|
| Inactive | 默认 | hitbox激活 | 不检测碰撞 |
| Detecting | hitbox启用 | overlap或超时 | 检测碰撞 |
| HitConfirmed | overlap确认 | 反馈序列完成 | 执行5步反馈 |
| Cooldown | 反馈完成 | 下一帧 | hitbox禁用 |

### Damage Calculation

```
final_damage = floor(base_damage * combo_multiplier * crit_multiplier)
```

- **combo_multiplier**: `1.0 + combo_count * 0.1`（每连击+10%）
- **crit_multiplier**: 暴击时1.5，否则1.0
- **crit_chance**: 15%（可由装备/技能修改）

### Feedback Layers (按触发顺序)

| Layer | Duration | What It Does | Priority |
|-------|----------|-------------|----------|
| 1. Hit-stop | 3-12帧 | Engine.time_scale = 0 | Critical |
| 2. Target deformation | 50-100ms | 挤压拉伸动画 | High |
| 3. Damage number | 800ms | 弹出浮动数字 | High |
| 4. Screen shake | 100-200ms | Camera偏移+衰减 | Medium |
| 5. Attacker deformation | 67ms | 攻击者轻微反向形变 | Low |

### Hit-Stop Table

| Attack Type | Hit-Stop Frames | Duration at 60fps | Notes |
|-------------|----------------|-------------------|-------|
| Light | 3 | 50ms | 极短，保持节奏 |
| Heavy | 8 | 133ms | 明显停顿，"砸"感 |
| Special | 12 | 200ms | 最长，戏剧性 |
| Crit (any) | +3 | +50ms | 暴击额外停顿 |
| Boss hit | +2 | +33ms | 打Boss更"沉" |

### Screen Shake Table

| Attack Type | Intensity (px) | Decay Rate | Duration |
|-------------|---------------|------------|----------|
| Light | 0 | — | 无震动 |
| Light (combo 3+) | 1.0 | 15/s | ~67ms |
| Heavy | 4.0 | 15/s | ~267ms |
| Special | 8.0 | 15/s | ~533ms |
| Crit (any) | +3.0 | — | 叠加 |

### Damage Number Style

| Attack Type | Font Size | Color | Outline | Extra |
|-------------|----------|-------|---------|-------|
| Light | 32px | #FFFFFF (白) | 3px黑 | 无 |
| Heavy | 42px | #FFD700 (金) | 3px黑 | 无 |
| Special | 48px | #FF4444 (红) | 3px黑 | 无 |
| Crit | 48px | #FFD700 (亮金) | 4px黑 | 后缀"!" |

### Invincibility Frames

| Trigger | I-Frame Duration | Visual Effect |
|---------|-----------------|---------------|
| Normal hit | 30 frames (500ms) | 8帧闪烁周期 |
| Heavy hit | 40 frames (667ms) | 更慢闪烁 |
| Special hit | 50 frames (833ms) | 更慢闪烁+红色叠加 |
| Dodge active | 8 frames (133ms) | 半透明+残影 |

### Interactions with Other Systems

| System | Interaction | Interface |
|--------|-------------|-----------|
| Combat System | 攻击激活hitbox | signal: hit_confirmed(target, damage, type) |
| Combo System | 连招计数影响伤害 | query: get_combo_count() |
| Player Controller | 被击→进入HitStun | method: enter_hit_stun(duration, knockback) |
| Enemy AI | 被击→受击行为 | method: take_damage(damage, type, is_crit) |
| Audio System | 命中音效 | signal: hit_sound_requested(type) |
| HUD | 伤害数字显示 | signal: damage_number_requested(pos, damage, type, crit) |

## Formulas

### Combo Damage Multiplier

```
combo_multiplier = 1.0 + combo_count * combo_bonus
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| combo_multiplier | float | 1.0-3.0 | calculated | 连招伤害倍率 |
| combo_count | int | 0-20 | combo system | 当前连击数 |
| combo_bonus | float | 0.05-0.15 | config | 每击加成比例 |

**Expected output range**: 1.0 (无连击) to 3.0 (20连击上限)
**Edge case**: combo_count > 20 时 clamp 到 3.0

### Crit Damage

```
crit_damage = floor(base_damage * combo_multiplier * 1.5)
```

**Expected output range**: 基础伤害的 1.5× - 4.5×

### Knockback Calculation

```
knockback = base_knockback * (1 + combo_count * 0.05) * direction
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| base_knockback | float | 50-200 | config | 基础击退(px/s) |
| direction | int | -1 or 1 | calculated | 攻击者面向 |
| combo_count | int | 0-20 | combo system | 连击加成 |

**Edge case**: 击退不超过屏幕宽度的30%（~384px），防止击出屏幕

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|----------|------------------|-----------|
| 攻击命中多个目标 | 每个目标独立触发反馈，但hit-stop只触发一次(取最大值) | 避免叠加停顿导致卡死 |
| 暴击+必杀同时 | 暴击优先显示(金色数字)，必杀震动强度叠加 | 暴击是更稀有的反馈 |
| 目标死亡时被命中 | 仍然触发完整反馈+击退，死亡动画延后 | 保证最后一击的爽感 |
| 两个攻击同时命中 | 按优先级处理(重攻>轻攻)，另一个排队等i-frame结束 | 防止伤害溢出 |
| i-frame期间再次被击 | 完全忽略，无视觉反馈 | 明确的无敌状态 |
| 命中停顿期间新输入 | 输入被缓冲，停顿结束后处理 | 不丢失玩家意图 |

## Dependencies

| System | Direction | Nature of Dependency |
|--------|-----------|---------------------|
| Player Controller | Bidirectional | 查询角色状态/通知受击 |
| Combat System | Combat depends on this | 提供命中结果和伤害 |
| Combo System | This depends on Combo | 查询连招计数影响伤害 |
| Enemy AI | This notifies Enemy | 命中事件通知敌人 |
| Audio System | This triggers Audio | 命中音效请求 |

## Tuning Knobs

| Parameter | Current Value | Safe Range | Effect of Increase | Effect of Decrease |
|-----------|--------------|------------|-------------------|-------------------|
| light_hit_stop | 3帧 | 1-6 | 更重但更慢 | 更快但更轻 |
| heavy_hit_stop | 8帧 | 4-14 | 重攻更"砸" | 重攻不够分量 |
| special_hit_stop | 12帧 | 6-20 | 必杀更戏剧 | 必杀不够震撼 |
| crit_bonus_hit_stop | 3帧 | 0-6 | 暴击更明显 | 暴击与普通差异小 |
| combo_bonus | 0.10 | 0.05-0.20 | 连击伤害增长更快 | 连击奖励更低 |
| crit_chance | 15% | 5-30% | 更频繁暴击 | 暴击更稀有 |
| crit_multiplier | 1.5x | 1.3-2.5 | 暴击伤害更高 | 暴击伤害更低 |
| i_frame_duration | 30帧 | 15-60 | 受击后无敌更长 | 更快可再次被击 |
| knockback_base | 200px/s | 100-400 | 击退更远 | 击退更近 |

## Visual/Audio Requirements

| Event | Visual Feedback | Audio Feedback | Priority |
|-------|----------------|---------------|----------|
| 轻攻命中 | 3帧停顿+白字 | 轻击打音效 | High |
| 重攻命中 | 8帧停顿+金字+中震+目标挤压 | 重击打音效 | High |
| 必杀命中 | 12帧停顿+红字+强震+目标强变形+全屏闪白 | 必杀命中音效 | Critical |
| 暴击 | +3帧停顿+金字+叹号+额外震 | 暴击音效(高亢) | Critical |
| 连击5+ | 屏幕轻微色差偏移 | 连击音效升调 | Medium |
| 未命中 | 无反馈(挥空) | 挥空音效(轻微) | Low |

## Game Feel

### Feel Reference
应像《任天堂明星大乱斗》的命中反馈——每次命中都有明确的"停顿→爆发"节奏。轻攻击是快速的点状停顿，重攻击是戏剧性的时间凝滞。暴击要有让玩家"哇"一声的冲击力。**不应像**早期横版游戏的无反馈命中——打过去敌人只是血条减少，画面没有任何回应。

### Impact Moments

| Impact Type | Duration (ms) | Effect Description | Configurable? |
|-------------|--------------|-------------------|---------------|
| Hit-stop (light) | 50ms | 全局时间冻结 | Yes |
| Hit-stop (heavy) | 133ms | 全局时间冻结+目标压缩 | Yes |
| Hit-stop (special) | 200ms | 全局时间冻结+全屏闪白 | Yes |
| Hit-stop (crit) | +50ms | 额外冻结叠加 | Yes |
| Screen shake (heavy) | 267ms | 方向性衰减震动 | Yes |
| Screen shake (special) | 533ms | 全方向强震 | Yes |
| Target squash | 50-100ms | 横向压缩→弹回 | Yes |
| Damage number rise | 800ms | 弹出→上升→淡出 | Yes |

### Weight and Responsiveness Profile

- **Weight**: 命中瞬间极重——时间冻结制造"此刻最重要"的感觉，然后爆发释放
- **Player control**: 命中停顿期间输入被缓冲不丢失，玩家永远不因反馈而丢失操控
- **Snap quality**: 停顿是突然的二元切换(time_scale 0→1)，不是渐变
- **Failure texture**: 未命中(wiff)有轻微挥空音效，与命中的强烈反馈形成对比，让玩家"知道没打中"

### Feel Acceptance Criteria

- [ ] 轻攻命中有可见的3帧停顿，测试者描述"打到了"而非"看不出来"
- [ ] 重攻命中有明显的8帧停顿+震动，测试者不自觉地说"哇"
- [ ] 暴击的视觉反馈与普通命中明确区分（金色+叹号+额外停顿）
- [ ] 连续5次命中后反馈强度递增明显
- [ ] 命中停顿期间玩家的下一个输入不丢失

## UI Requirements

| Information | Display Location | Update Frequency | Condition |
|-------------|-----------------|-----------------|-----------|
| 伤害数字 | 命中位置上方 | 事件触发 | 每次命中 |
| 连击数 | 屏幕中上方 | 每次命中 | combo>0 |
| 目标血条 | 目标头顶 | 每帧 | 战斗中 |

## Cross-References

| This Document References | Target GDD | Specific Element Referenced | Nature |
|--------------------------|-----------|----------------------------|--------|
| combo_count影响伤害 | `design/gdd/combo-system.md` | combo_count值 | Data dependency |
| 被击进入HitStun | `design/gdd/player-controller.md` | enter_hit_stun() | State trigger |
| hit-stop实现方式 | `prototypes/combat-concept/hit_stop_manager.gd` | apply_hit_stop() | Rule dependency |
| 伤害数字颜色编码 | `design/art/art-bible.md` | VFX颜色规范 | Rule dependency |
| 屏幕震动参数 | `prototypes/combat-concept/screen_shake.gd` | shake() | Rule dependency |

## Acceptance Criteria

- [ ] 轻攻/重攻/必杀命中分别触发3/8/12帧hit-stop
- [ ] 暴击15%概率，1.5x伤害，额外3帧hit-stop+金色数字+叹号
- [ ] 伤害数字颜色按攻击类型编码(白/金/红)
- [ ] i-frame期间不会受到同一攻击的二次伤害
- [ ] 连招伤害递增(每击+10%)，上限3x
- [ ] 命中停顿期间输入正确缓冲不丢失
- [ ] Performance: 命中反馈序列在1帧内完成初始化
- [ ] No hardcoded values in implementation

## Open Questions

| Question | Owner | Deadline | Resolution |
|----------|-------|----------|-----------|
| 命中音效是否按材质分？ | sound-designer | Audio System GDD时 | 倾向按攻击类型分，不按材质 |
| 多目标AOE的hit-stop叠加？ | game-designer | GDD审批时 | 取最大值不叠加 |
