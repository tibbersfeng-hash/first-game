# Dungeon Room System

> **Status**: Draft
> **Author**: game-designer
> **Last Updated**: 2026-06-15
> **Last Verified**: —
> **Implements Pillar**: 爽快打击+Q版反差 / 职业深度与成长 (房间是战斗的舞台)

## Summary

副本房间系统管理单个房间的布局、敌人波次、边界锁定和清理条件。它是"清怪→开门→下一间"循环的实现者——玩家进入房间，门锁上，清完敌人，门打开，奖励出现。每个房间是一次完整的战斗体验，有开始、高潮和结束。

> **Quick reference** — Layer: `Presentation` · Priority: `MVP` · Key deps: `Enemy AI`

## Overview

Dungeon Room 定义了"一个房间"的一切：物理布局（平台/墙壁/装饰）、敌人配置（类型/数量/波次）、战斗区域边界（锁定门）、清理条件和奖励。房间是 Dungeon Flow 的基本单位——整个副本就是一系列房间的有序组合。

## Player Fantasy

推开一扇门，走进一间暗室，身后门锁上，前方涌出敌人。心跳加速，连招开始。最后一个敌人倒下，房间亮起，宝箱出现。"再来一间"的冲动就是房间系统的设计目标。

## Detailed Design

### Core Rules

1. **房间锁定**: 所有敌人清除前，出口关闭且不可通过
2. **波次系统**: 房间可有1-3波敌人，每波清除后触发下一波
3. **波间间隔**: 波次之间有2秒安全窗口（无敌人+恢复时间）
4. **清理条件**: 当前波所有敌人HP≤0
5. **房间完成**: 所有波次清除→门开+奖励生成+评价显示
6. **房间尺寸**: 固定1280×720像素（一个屏幕宽度，无滚动）

### Room Layout Components

| Component | Purpose | Config |
|-----------|---------|--------|
| 地面 | 角色站立/移动 | 1-3段，高度可变 |
| 平台 | 空中战斗/跳跃路线 | 0-4个，位置可配 |
| 墙壁 | 左右边界 | 固定2面 |
| 天花板 | 上边界+跳跃限制 | 固定1面 |
| 门(入口) | 进入房间 | 左侧或右侧1个 |
| 门(出口) | 离开房间 | 与入口对面 |
| 装饰物 | 氛围+可破坏物 | 0-5个，可掉落金币 |

### Wave Configuration

```gdscript
# Room配置示例
@export var room_id: String
@export var theme: String  # "cave", "castle", "lava", "ice"
@export var waves: Array[WaveConfig]
@export var platforms: Array[PlatformConfig]
@export var destructibles: Array[DestructibleConfig]

# Wave配置
@export var enemies: Array[EnemySpawnConfig]  # 敌人生成列表
@export var spawn_delay: float  # 生成延迟(秒)
@export var spawn_pattern: String  # "all_at_once" / "staggered" / "reinforcement"

# EnemySpawn配置
@export var enemy_type: String  # "minion_goblin", "elite_skeleton", etc.
@export var count: int
@export var spawn_point: Vector2  # 生成位置
@export var spawn_delay: float  # 个体延迟
```

### Spawn Patterns

| Pattern | Description | Use Case |
|---------|-------------|----------|
| all_at_once | 所有敌人同时出现 | 小怪房间，立即战斗 |
| staggered | 每0.5秒生成一个 | 中等难度，节奏可控 |
| reinforcement | 当前敌人<50%时生成增援 | 持续压力，不允许喘息 |

### Room Lifecycle States

| State | Entry Condition | Exit Condition | Player Can | Notes |
|-------|----------------|----------------|-----------|-------|
| Entering | 玩家进入房间 | 进入动画完成 | 移动 | 入场动画0.5s |
| Active | 进入完成 | 所有波次清除 | 战斗/移动 | 门锁定 |
| WaveCleared | 当前波清除 | 下一波开始/房间完成 | 移动/拾取 | 2s安全窗口 |
| Completed | 所有波清除 | 玩家离开 | 拾取/移动 | 门解锁+奖励 |
| Exiting | 玩家走到出口 | 离开动画完成 | 移动 | 出场动画0.5s |

### Room Rating (S/A/B/C)

| Rating | Condition | Reward Multiplier | Visual |
|--------|-----------|-------------------|--------|
| S | 无伤 + 20连击+ | 2.0x | 金色+特效 |
| A | 无伤 或 15连击+ | 1.5x | 白色 |
| B | 受伤<30%HP | 1.2x | 蓝色 |
| C | 受伤>30%HP | 1.0x | 灰色 |

### Room Size and Camera

- 房间尺寸: 1280×720px（固定，等于视口大小）
- 摄像机: 不跟随角色移动，固定在房间中心
- 无需滚动: 所有内容在一屏内
- 平台布局: 必须保证从地面可达所有平台

### Interactions with Other Systems

| System | Interaction | Interface |
|--------|-------------|-----------|
| Enemy AI | 管理敌人生成/死亡 | spawn_enemy(type, pos), signal: enemy_died() |
| Dungeon Flow | 房间是Dungeon Flow的基本单位 | signal: room_cleared(rating) |
| HUD | 显示房间进度/波次/评分 | signal: wave_changed(current, total) |
| Combat System | 战斗在房间内发生 | 无直接接口，通过Enemy间接 |
| Player Controller | 玩家在房间内移动 | clamp_position_to_room() |

## Formulas

### Wave Spawn Timing (staggered pattern)

```
spawn_time = wave_start + index * stagger_delay + random_offset
random_offset = randf_range(0, stagger_delay * 0.3)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| wave_start | float | — | state | 波次开始时间 |
| index | int | 0-10 | loop | 敌人索引 |
| stagger_delay | float | 0.3-1.0 | config | 间隔延迟 |
| random_offset | float | 0-0.3 | random | 避免完全均匀 |

### Room Rating Calculation

```
if no_damage_taken and combo_count >= 20:
    rating = S, multiplier = 2.0
elif no_damage_taken or combo_count >= 15:
    rating = A, multiplier = 1.5
elif damage_taken < max_hp * 0.3:
    rating = B, multiplier = 1.2
else:
    rating = C, multiplier = 1.0
```

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|----------|------------------|-----------|
| 玩家在波次间隔中走到出口 | 出口仍锁，等所有波次清除 | 不允许跳过 |
| 最后一波敌人掉出平台 | 计为死亡，波次清除 | 横版格斗传统 |
| 房间内所有敌人都卡住 | 5秒无伤害→敌人自动死亡+无评分 | 安全网 |
| 玩家在Entering时被击 | Entering期间无敌 | 入场保护 |
| 房间完成但玩家不离开 | 无时间限制，可自由拾取 | 不催促 |
| 波次中新增敌人生成失败 | 日志警告+跳过该敌人 | 容错处理 |

## Dependencies

| System | Direction | Nature of Dependency |
|--------|-----------|---------------------|
| Enemy AI | This manages Enemies | 生成/清除/死亡监听 |
| Dungeon Flow | Flow manages Rooms | 房间顺序/进入/离开 |
| HUD | HUD depends on this | 显示房间状态 |
| Player Controller | This constrains Player | 限制在房间边界内 |

## Tuning Knobs

| Parameter | Current Value | Safe Range | Effect of Increase | Effect of Decrease |
|-----------|--------------|------------|-------------------|-------------------|
| wave_interval | 2.0s | 1.0-4.0 | 更多恢复时间 | 更少喘息 |
| stagger_delay | 0.5s | 0.3-1.5 | 敌人出现更分散 | 敌人更快涌出 |
| max_waves_per_room | 3 | 1-5 | 房间更长更难 | 房间更短更简单 |
| room_width | 1280px | 960-1920 | 更大战斗空间 | 更紧凑 |
| room_height | 720px | 540-1080 | 更高跳跃空间 | 更矮 |
| s_rank_combo_threshold | 20 | 10-30 | S评更难 | S评更容易 |
| a_rank_combo_threshold | 15 | 8-20 | A评更难 | A评更容易 |
| stuck_timeout | 5.0s | 3.0-15.0 | 更长等待 | 更快安全网 |

## Visual/Audio Requirements

| Event | Visual Feedback | Audio Feedback | Priority |
|-------|----------------|---------------|----------|
| 房间锁定 | 门关闭动画+锁链特效 | 锁门声 | High |
| 新波次 | 屏幕边缘闪烁+敌人生成特效 | 警报声 | High |
| 波次清除 | 短暂慢动作+闪光 | 清除音效 | Medium |
| 房间完成 | 全屏亮起+门开+奖励出现 | 通关音乐 | High |
| S评价 | 金色大字+粒子爆炸 | 评价音效(高亢) | Critical |
| 生成敌人 | 传送门特效(0.5s) | 生成音效 | Medium |

## Game Feel

### Feel Reference
应像《龙王战士》的房间结构——进入→锁门→清怪→开门，简单但有效。每间房是一次完整的情绪弧：紧张(锁门)→高潮(战斗)→释放(清除)→奖励(宝箱)。**不应像**《以撒的结合》的无限房间——那缺乏节奏感。

### Feel Acceptance Criteria

- [ ] 锁门→清怪→开门循环感觉自然，无测试者困惑"为什么门不开"
- [ ] 波次间隔2秒有足够恢复感但不过长
- [ ] S评价视觉反馈让玩家想截图分享
- [ ] 所有平台从地面可达，无死路
- [ ] 房间完成时的亮起动画有成就感

## UI Requirements

| Information | Display Location | Update Frequency | Condition |
|-------------|-----------------|-----------------|-----------|
| 波次进度 | 屏幕右上 | 波次变化时 | 战斗中 |
| 房间评分 | 屏幕中央 | 房间完成时 | 2秒显示 |
| 敌人剩余数 | 屏幕右上 | 敌人死亡时 | 战斗中 |
| 锁定状态 | 出口门图标 | 状态变化时 | 房间内 |

## Cross-References

| This Document References | Target GDD | Specific Element Referenced | Nature |
|--------------------------|-----------|----------------------------|--------|
| 敌人生成/死亡 | `design/gdd/enemy-ai.md` | enemy_spawn/death | Ownership handoff |
| 房间组成Dungeon Flow | `design/gdd/dungeon-flow.md` | room_sequence | Data dependency |
| 连击数影响评分 | `design/gdd/combo-system.md` | combo_count | Data dependency |
| 房间视觉主题 | `design/art/art-bible.md` | 地牢主题色板 | Rule dependency |

## Acceptance Criteria

- [ ] 房间5状态生命周期正确流转（Entering→Active→WaveCleared→Completed→Exiting）
- [ ] 所有敌人清除前出口锁定且不可通过
- [ ] 波次间隔2秒，波间无敌人
- [ ] 3种生成模式(all_at_once/staggered/reinforcement)正确工作
- [ ] 评分S/A/B/C按公式正确计算
- [ ] 房间尺寸1280×720，摄像机固定
- [ ] 卡敌安全网：5秒无伤害自动清除+无评分
- [ ] Performance: 房间内10敌人时保持60fps
- [ ] No hardcoded values in implementation

## Open Questions

| Question | Owner | Deadline | Resolution |
|----------|-------|----------|-----------|
| 房间是否有陷阱/机关？ | game-designer | VS阶段 | MVP不做，纯战斗房间 |
| 可破坏环境物？ | game-designer | GDD审批时 | 倾向做，增加互动感 |
| 房间间是否有走廊？ | game-designer | Dungeon Flow GDD时 | 短走廊作为过渡 |
