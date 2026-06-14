# Enemy AI

> **Status**: Draft
> **Author**: ai-programmer + game-designer
> **Last Updated**: 2026-06-15
> **Last Verified**: —
> **Implements Pillar**: 爽快打击+Q版反差 / 职业深度与成长 (敌人是打击感的受体和成长的标尺)

## Summary

敌人AI系统管理所有非玩家角色的行为决策、状态机和攻击模式。它提供通用AI框架（用于小怪）和Boss专用行为树（用于Boss），通过数据驱动的配置表定义不同敌人的行为。核心目标是让小怪"够蠢够爽"（让玩家觉得强），让Boss"够难够记忆"（需要学习但可战胜）。

> **Quick reference** — Layer: `Feature` · Priority: `MVP` · Key deps: `Combat System, Hit Detection`

## Overview

Enemy AI 分为两个层级：通用小怪AI（基于简单状态机+决策间隔）和Boss AI（基于行为树+阶段切换）。所有敌人的行为参数来自配置文件而非硬编码，允许设计师无需改代码调整难度。敌人必须"可读"——玩家能通过动画预判攻击，而非被突然打中。

## Player Fantasy

小怪像沙袋——站着让你打，偶尔反击让你注意走位，打起来爽但有节奏。Boss像对手——每次出招都有预兆，你需要学习和记忆，成功闪避后的反击比打小怪爽10倍。Q版敌人被打时的夸张反应让杀敌本身变成奖励。

## Detailed Design

### Core Rules

1. **决策间隔**: 敌人每隔decision_interval秒做一次决策，不是每帧（性能+可读性）
2. **可读性优先**: 所有攻击必须有明确预兆动画（telegraph），minimum_telegraph_frames帧
3. **数据驱动**: 行为参数全部来自EnemyData Resource，不硬编码
4. **难度缩放**: 同类敌人可通过参数调整难度，不改逻辑
5. **群体协作**: 多敌人时自动避免同时攻击（attack_slot机制）

### Enemy Categories

| Category | HP Ratio | Decision Speed | Attack Slots | Behavior |
|----------|---------|---------------|-------------|----------|
| 小怪(Minion) | 1x | 慢(1.0-2.0s) | 不限制 | 简单巡逻→发现→攻击→死亡 |
| 精英(Elite) | 3x | 中(0.5-1.0s) | 不限制 | 小怪行为+特殊技能+护甲 |
| Boss | 10-30x | 快(0.3-0.8s) | 独占 | 阶段切换+多技能组合 |

### Minion AI State Machine

| State | Entry Condition | Exit Condition | Behavior |
|-------|----------------|----------------|----------|
| Idle | 默认/巡逻结束 | 发现玩家 | 待机或缓慢巡逻 |
| Patrol | 巡逻点未到 | 到达巡逻点/发现玩家 | 沿巡逻点移动 |
| Alert | 发现玩家(视野内) | 丢失目标/进入攻击范围 | 面向玩家+警戒动画 |
| Chase | 目标在追击范围外 | 进入攻击范围/丢失目标 | 向玩家移动 |
| Attack | 目标在攻击范围内 | 攻击完成 | 执行攻击动作 |
| HitStun | 被击中 | 硬直结束 | 受击动画 |
| Death | HP≤0 | — | 死亡动画+掉落 |

### Minion Decision Logic

每次decision_interval触发时：

```
1. If in HitStun/Death → skip decision
2. If player in attack_range → Attack
3. If player in chase_range → Chase
4. If player in alert_range → Alert
5. Else → Patrol or Idle
```

### Boss AI — Phase System

Boss有多个阶段，每个阶段有不同的行为模式：

| Phase | HP Threshold | Behavior Change | New Attacks |
|-------|-------------|----------------|-------------|
| Phase 1 | 100%-60% | 基础模式 | 攻击A + 攻击B |
| Phase 2 | 60%-30% | 变速+新增攻击 | + 攻击C(范围) + 攻击D(连击) |
| Phase 3 | 30%-0% | 狂暴模式 | + 攻击E(必杀) + 速度×1.3 |

**Phase Transition**: HP跨过阈值时播放转场动画（2秒无敌），然后进入新阶段。

### Boss Attack Pattern

Boss攻击由序列模式定义：

```
pattern = [attackA, wait, attackB, wait, attackC, long_wait, ...]
```

- 每个pattern是一轮攻击循环
- wait时间随阶段缩短
- 部分攻击有条件触发（如HP<50%时才用）
- 玩家距离影响攻击选择（近战/远程攻击）

### Attack Slot System

防止多敌人同时攻击造成不公平：

| Rule | Detail |
|------|--------|
| 同时攻击上限 | 2个敌人 |
| 攻击申请 | 敌人想攻击时请求slot |
| 申请失败 | 改为Chase（等slot释放） |
| Boss | 独占，不计入slot限制 |
| Slot释放 | 攻击recovery结束后释放 |

### Telegraph System (可读性保证)

| Attack Tier | Minimum Telegraph | Visual Cue | Audio Cue |
|-------------|-----------------|------------|-----------|
| 小怪轻攻 | 8帧(133ms) | 短暂蓄力姿势 | 无 |
| 小怪重攻 | 15帧(250ms) | 明显蓄力+红色闪光 | 低沉蓄力声 |
| Boss普通 | 20帧(333ms) | 蓄力姿态+特效 | 蓄力音效 |
| Boss大招 | 45帧(750ms) | 长蓄力+全屏闪烁+警告线 | 强蓄力音效+警报 |

### Interactions with Other Systems

| System | Interaction | Interface |
|--------|-------------|-----------|
| Combat System | 敌人攻击使用相同伤害/取消规则 | enemy.attack(type) |
| Hit Detection | 被命中时触发受击 | take_damage(damage, type, is_crit) |
| Dungeon Room | 房间管理敌人激活/清理 | signal: enemy_died(enemy) |
| Loot & Drop | 死亡时掉落物品 | signal: loot_dropped(items) |
| Audio | 攻击/受击/死亡音效 | signal: enemy_sound(type) |

## Formulas

### Decision Interval with Distance

```
interval = base_interval * (1 + distance_factor * player_distance / max_distance)
interval = clamp(interval, min_interval, max_interval)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| base_interval | float | 0.5-2.0 | config | 基础决策间隔(秒) |
| distance_factor | float | 0.0-1.0 | config | 距离对决策速度影响 |
| player_distance | float | 0-500 | calculated | 到玩家的距离 |
| max_distance | float | 500 | config | 最大影响距离 |

**Edge case**: 距离越远决策越慢，防止远处敌人疯狂决策浪费性能

### Boss Phase Speed Multiplier

```
speed_mult = 1.0 + (phase_index - 1) * phase_speed_bonus
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| phase_index | int | 1-3 | state | 当前阶段 |
| phase_speed_bonus | float | 0.2-0.5 | config | 每阶段速度加成 |

**Expected**: Phase1=1.0x, Phase2=1.3x, Phase3=1.6x

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|----------|------------------|-----------|
| 玩家离开敌人追击范围 | 敌人回到巡逻点，不无限追 | 防止全图拉怪 |
| Boss转场动画中被攻击 | Boss无敌，伤害无效 | 转场是叙事时刻 |
| 所有攻击slot被占 | 等待的敌人保持Chase | 不造成不公平多打一 |
| 敌人被击飞出平台 | 掉落+直接死亡 | 横版格斗经典机制 |
| 玩家隐身/不可见 | 敌人回到Alert状态巡逻 | 不追看不见的目标 |
| Boss Phase 3 HP从35%直接跳到25% | 仍然触发Phase 2→3转场 | 不跳过阶段 |

## Dependencies

| System | Direction | Nature of Dependency |
|--------|-----------|---------------------|
| Combat System | This depends on Combat | 使用攻击类型和伤害规则 |
| Hit Detection | Bidirectional | 被命中→受击/命中→通知 |
| Dungeon Room | Room manages Enemies | 激活/清理/掉落 |
| Loot & Drop | This triggers Loot | 死亡时掉落 |

## Tuning Knobs

| Parameter | Current Value | Safe Range | Effect of Increase | Effect of Decrease |
|-----------|--------------|------------|-------------------|-------------------|
| minion_decision_interval | 1.5s | 0.5-3.0 | 小怪更迟钝 | 小怪更敏捷 |
| minion_attack_range | 80px | 50-150 | 小怪更早攻击 | 需要更近 |
| minion_chase_range | 300px | 150-500 | 追更远 | 更快放弃追击 |
| max_attack_slots | 2 | 1-4 | 更多多同时攻击 | 更宽松 |
| boss_phase_speed_bonus | 0.3 | 0.1-0.5 | Boss后期更快 | Boss后期仍慢 |
| telegraph_frames_min | 8 | 4-20 | 攻击更可读 | 攻击更突然 |
| boss_phase2_threshold | 60% | 40-80 | 更早进入P2 | 更晚进入P2 |
| boss_phase3_threshold | 30% | 15-50 | 更早狂暴 | 更晚狂暴 |

## Visual/Audio Requirements

| Event | Visual Feedback | Audio Feedback | Priority |
|-------|----------------|---------------|----------|
| 小怪发现玩家 | 头顶感叹号 | 警戒声 | Medium |
| 攻击预兆 | 蓄力姿态+红色闪光 | 蓄力音效 | High |
| 被命中 | Q版挤压+表情变化 | 受击声 | High |
| 死亡 | 夸张倒地+灰尘+弹跳 | 死亡声 | High |
| Boss转场 | 全屏特效+无敌光环 | 转场音效 | Critical |
| Boss新阶段 | 外观变化+咆哮 | 咆哮声 | Critical |
| 精英护甲破损 | 碎片飞溅 | 破碎声 | Medium |

## Game Feel

### Feel Reference
小怪应像《龙王战士》的小兵——傻但数量多，打起来爽且有节奏。Boss应像《胧村正》的Boss——每个攻击都有预兆，学会后能完美闪避+反击，感觉自己是高手。**不应像**魂系游戏的"突然出招"——那在横版格斗中是设计失败。

### Feel Acceptance Criteria

- [ ] 小怪攻击8帧预兆清晰可见，玩家可在看到后闪避
- [ ] Boss攻击45帧预兆，玩家有充足反应时间
- [ ] 同时不超过2个小怪攻击，玩家不会感觉"不公平"
- [ ] Boss转场2秒无敌动画戏剧感强
- [ ] Q版敌人受击的挤压变形让玩家想"再多打一下"

## UI Requirements

| Information | Display Location | Update Frequency | Condition |
|-------------|-----------------|-----------------|-----------|
| 敌人HP条 | 敌人头顶 | 每帧 | 受到伤害时 |
| Boss HP条 | 屏幕底部大条 | 每帧 | Boss战 |
| Boss阶段 | HP条分段标记 | 阶段切换时 | Boss战 |

## Cross-References

| This Document References | Target GDD | Specific Element Referenced | Nature |
|--------------------------|-----------|----------------------------|--------|
| 敌人攻击使用战斗规则 | `design/gdd/combat-system.md` | 攻击帧数据/伤害公式 | Rule dependency |
| 敌人被命中反馈 | `design/gdd/hit-detection-feedback.md` | take_damage() | State trigger |
| 房间管理敌人 | `design/gdd/dungeon-room.md` | enemy_spawn/enemy_died | Ownership handoff |
| 敌人视觉风格 | `design/art/art-bible.md` | 角色美术标准 | Rule dependency |

## Acceptance Criteria

- [ ] 小怪6状态机正确转换，无死锁状态
- [ ] Boss 3阶段正确切换，转场动画2秒无敌
- [ ] 攻击slot系统限制同时攻击≤2
- [ ] 所有攻击预兆≥8帧(小怪)/≥20帧(Boss普通)/≥45帧(Boss大招)
- [ ] 决策间隔按公式正确计算
- [ ] 数据驱动：EnemyData Resource配置可替换，无硬编码行为
- [ ] Performance: 单敌人AI update < 0.1ms，10敌人 < 0.5ms
- [ ] No hardcoded values in implementation

## Open Questions

| Question | Owner | Deadline | Resolution |
|----------|-------|----------|-----------|
| 是否需要敌人巡逻路径编辑器？ | game-designer | Dungeon Room GDD时 | MVP手动配置巡逻点 |
| Boss是否有狂暴计时？ | game-designer | GDD审批时 | 倾向不做，避免压力感 |
| 精英敌人特殊技能？ | game-designer | VS阶段 | MVP精英=强化版小怪 |
