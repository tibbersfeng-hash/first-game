# Dungeon Flow System

> **Status**: Draft
> **Author**: game-designer
> **Last Updated**: 2026-06-15
> **Last Verified**: —
> **Implements Pillar**: 爽快打击+Q版反差 (副本流程是战斗体验的节奏控制器)

## Summary

副本流程系统管理从"选择副本→逐房间推进→Boss战→结算"的完整流程。它编排房间序列、房间间过渡、Boss战触发和最终结算画面。它是房间级的"导演"——Dungeon Room 决定一间房怎么打，Dungeon Flow 决定整个副本怎么走。

> **Quick reference** — Layer: `Presentation` · Priority: `MVP` · Key deps: `Dungeon Room System`

## Overview

Dungeon Flow 是玩家体验"一局游戏"的完整弧线：城镇整备→选副本→房间1→房间2→...→Boss房→结算→回城镇。它定义副本的长度、难度曲线、Boss位置和奖励结算。每个副本是一个独立的完整体验，像一集动画——有开头、发展、高潮和结尾。

## Player Fantasy

选一个副本，知道大概要打多久（5-15分钟），体验难度逐渐攀升的紧张感，Boss战是高潮，赢了有成就感，输了想再来一次。副本长度不会让人累，短副本可以"再来一局"。

## Detailed Design

### Core Rules

1. **线性流程**: 房间按预设顺序排列，无分支（MVP简化）
2. **难度曲线**: 房间难度逐步递增，Boss是最高峰
3. **复活机制**: 每个副本有复活币上限，用完则副本失败
4. **结算画面**: 副本完成/失败后显示统计和评分
5. **中途退出**: 可随时退出，但不获得任何奖励
6. **副本时长**: 短(5间/5min) / 中(8间/10min) / 长(12间/15min)

### Dungeon Structure

```
入口走廊 → 房间1(小怪) → 走廊 → 房间2(小怪+精英) → 走廊 → 房间3(Boss) → 结算
```

每个副本由以下节点组成：

| Node Type | Purpose | Duration | Combat |
|-----------|---------|----------|--------|
| Entrance | 副本入口+氛围建立 | 5s | 无 |
| Corridor | 房间间过渡+恢复 | 3-5s | 无 |
| Combat Room | 战斗房间 | 30-90s | 有(1-3波) |
| Boss Room | Boss战 | 60-180s | 有(Boss) |
| Reward Room | 结算+拾取 | 10-20s | 无 |

### Dungeon Template Definition

```gdscript
@export var dungeon_id: String
@export var dungeon_name: String
@export var theme: String  # "cave", "castle", "lava", "ice"
@export var difficulty: String  # "normal", "hard", "hell"
@export var revive_coins: int  # 复活币数量
@export var rooms: Array[String]  # 房间ID序列
@export var boss_id: String  # Boss房间ID
@export var recommended_level: int
@export var time_limit: float  # 秒，0=无限制
```

### Difficulty Scaling

| Difficulty | Enemy HP | Enemy Damage | Revive Coins | Telegraph Speed | Room Count |
|-----------|---------|-------------|-------------|----------------|-----------|
| Normal | 1.0x | 1.0x | 3 | 1.0x | 5-8 |
| Hard | 1.5x | 1.3x | 2 | 0.8x | 8-10 |
| Hell | 2.0x | 1.6x | 1 | 0.6x | 10-12 |

### Revive System

| Rule | Detail |
|------|--------|
| 复活币 | 每个副本有限数量(Normal=3, Hard=2, Hell=1) |
| 死亡触发 | HP≤0时弹出复活选择界面 |
| 选择复活 | 消耗1枚币，原地满血复活+3秒无敌 |
| 选择放弃 | 副本失败，回到城镇，无奖励 |
| 用完币后死亡 | 自动副本失败 |
| 房间完成时恢复 | 每完成一个房间恢复10%HP |

### States and Transitions

| State | Entry Condition | Exit Condition | Player Can |
|-------|----------------|----------------|-----------|
| DungeonSelect | 选择副本界面 | 确认选择 | 浏览副本列表 |
| DungeonEnter | 进入副本 | 入场动画完成 | 观看 |
| Corridor | 房间间过渡 | 到达下一房间 | 移动(单向) |
| RoomActive | 进入战斗房间 | 房间清除 | 战斗 |
| BossIntro | 进入Boss房 | Boss出场动画完成 | 观看 |
| BossActive | Boss战开始 | Boss击败/玩家死亡 | 战斗 |
| DungeonClear | 副本通关 | 结算完成 | 拾取/查看统计 |
| DungeonFail | 副本失败 | 确认 | 查看统计/重试 |
| DungeonExit | 离开副本 | 回到城镇 | 无 |

### Settlement Screen (结算画面)

| Stat | Description |
|------|-------------|
| 通关时间 | 从进入副本到Boss击败的总时间 |
| 总连击数 | 整个副本的连击累计 |
| 最高连击 | 单次最高连击数 |
| 总伤害 | 玩家造成的总伤害 |
| 受伤次数 | 被击中次数 |
| 使用复活币 | 消耗的复活币数量 |
| 综合评分 | S/A/B/C（基于所有房间评分的加权平均） |
| 获得经验 | 基础×评分倍率 |
| 获得金币 | 基础×评分倍率 |
| 获得装备 | 按掉落表随机 |

### MVP Dungeon List

| Dungeon | Theme | Rooms | Boss | Difficulty | Est. Time |
|---------|-------|-------|------|-----------|-----------|
| 哥布林洞穴 | Cave | 5 | 哥布林王 | Normal | 5min |
| 幽灵古堡 | Castle | 8 | 亡灵领主 | Normal/Hard | 10min |
| 熔岩地狱 | Lava | 10 | 炎魔将军 | Hard/Hell | 12min |

### Interactions with Other Systems

| System | Interaction | Interface |
|--------|-------------|-----------|
| Dungeon Room | Flow管理Room序列 | load_room(room_id), signal: room_cleared(rating) |
| Enemy AI | Boss战特殊触发 | signal: boss_defeated() |
| HUD | 显示副本进度/复活币 | signal: dungeon_state_changed(state) |
| Character Stats | 经验/金币结算 | add_experience(amount), add_gold(amount) |
| Equipment System | 装备掉落 | signal: loot_dropped(items) |
| Town Hub | 从城镇选择副本 | start_dungeon(dungeon_id) |

## Formulas

### Overall Dungeon Rating

```
avg_rating = weighted_average(room_ratings)
weights: combat_room=1.0, boss_room=2.0
```

| Rating | Score Range | Multiplier |
|--------|-----------|-----------|
| S | avg ≥ 3.5 | 2.0x |
| A | avg ≥ 2.5 | 1.5x |
| B | avg ≥ 1.5 | 1.2x |
| C | avg < 1.5 | 1.0x |

(S=4分, A=3分, B=2分, C=1分)

### Experience Reward

```
exp = base_exp * difficulty_multiplier * rating_multiplier * (1 + speed_bonus)
speed_bonus = max(0, (par_time - actual_time) / par_time * 0.3)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| base_exp | int | 100-500 | config | 副本基础经验 |
| difficulty_multiplier | float | 1.0-2.0 | config | 难度加成 |
| rating_multiplier | float | 1.0-2.0 | calculated | 评分加成 |
| speed_bonus | float | 0-0.3 | calculated | 速通奖励 |
| par_time | float | 300-900 | config | 标准时间(秒) |

### Gold Reward

```
gold = base_gold * difficulty_multiplier * rating_multiplier + room_gold_total
```

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|----------|------------------|-----------|
| 副本中网络断开(未来PVP) | 本地副本无影响 | MVP无网络需求 |
| Boss战最后一击同时玩家死亡 | 先判定Boss死亡→通关 | 玩家优先 |
| 所有复活币用完后死亡 | 自动副本失败 | 清晰的资源限制 |
| 速通时间<50%标准时间 | 额外30%经验奖励 | 奖励高手 |
| 副本超时(time_limit>0) | 副本失败 | 防止挂机 |
| 中途退出 | 无奖励，回到城镇 | 退出有代价 |

## Dependencies

| System | Direction | Nature of Dependency |
|--------|-----------|---------------------|
| Dungeon Room | This manages Rooms | 房间序列+过渡 |
| Enemy AI | Boss defeat triggers clear | Boss战结束 |
| HUD | HUD depends on this | 显示副本状态 |
| Town Hub | This is triggered from Town | 选择副本 |
| Character Stats | This rewards Stats | 经验/金币 |

## Tuning Knobs

| Parameter | Current Value | Safe Range | Effect of Increase | Effect of Decrease |
|-----------|--------------|------------|-------------------|-------------------|
| revive_coins_normal | 3 | 1-5 | 更宽容 | 更硬核 |
| revive_coins_hard | 2 | 1-3 | 更宽容 | 更硬核 |
| revive_coins_hell | 1 | 0-2 | 更宽容 | 极硬核 |
| hp_recover_per_room | 10% | 0-30% | 房间间更多恢复 | 更持续压力 |
| difficulty_hp_scale | 1.5x(Hard) | 1.0-3.0 | 敌人更肉 | 敌人更脆 |
| difficulty_dmg_scale | 1.3x(Hard) | 1.0-2.0 | 敌人更痛 | 敌人更轻 |
| speed_bonus_cap | 0.3 | 0.1-0.5 | 速通奖励更高 | 速通奖励更低 |
| time_limit | 0(无限) | 0-1800 | 限时更紧 | 无限时 |

## Visual/Audio Requirements

| Event | Visual Feedback | Audio Feedback | Priority |
|-------|----------------|---------------|----------|
| 副本选择 | 副本卡片+难度标识 | 选择音效 | Medium |
| 进入副本 | 场景淡入+氛围建立 | 副本主题BGM开始 | High |
| 走廊过渡 | 场景切换+脚步声 | 走廊环境音 | Low |
| Boss出场 | 全屏变暗+聚光灯+Boss特写 | Boss主题音乐 | Critical |
| 通关 | 全屏金光+烟花+统计浮现 | 通关胜利音乐 | Critical |
| 失败 | 画面灰化+倒地 | 失败音乐 | High |
| 复活选择 | 弹窗+角色闪烁 | 复活音效 | High |

## Game Feel

### Feel Reference
应像《DNF》的副本流程——选图→推图→Boss→翻牌，节奏紧凑，每局10-15分钟。走廊是呼吸，Boss是高潮，结算是奖励。**不应像**《黑暗之魂》的一次性通关——那不适合"再来一局"的循环。

### Feel Acceptance Criteria

- [ ] 从选副本到开始战斗不超过10秒
- [ ] 走廊过渡自然，不感觉"加载中"
- [ ] Boss出场有戏剧性，测试者能感受到"大的来了"
- [ ] 通关结算让玩家想截图分享
- [ ] 失败后想"再来一次"而非"太烦了"

## UI Requirements

| Information | Display Location | Update Frequency | Condition |
|-------------|-----------------|-----------------|-----------|
| 房间进度 | 右上角 (Room 3/8) | 房间切换时 | 副本中 |
| 复活币剩余 | HP条旁 | 消耗时 | 副本中 |
| 副本计时 | 右上角 | 每秒 | 副本中 |
| Boss HP | 屏幕底部大条 | 每帧 | Boss战 |
| 结算统计 | 屏幕中央 | 结算时 | 通关/失败 |

## Cross-References

| This Document References | Target GDD | Specific Element Referenced | Nature |
|--------------------------|-----------|----------------------------|--------|
| 房间序列由Room组成 | `design/gdd/dungeon-room.md` | room_id序列 | Data dependency |
| Boss击败触发通关 | `design/gdd/enemy-ai.md` | Boss Phase System | State trigger |
| 房间评分汇总结算 | `design/gdd/dungeon-room.md` | room_rating | Data dependency |
| 经验/金币结算 | `design/gdd/character-stats.md` (VS) | add_experience() | Ownership handoff |

## Acceptance Criteria

- [ ] 3个MVP副本可从选择到通关完整走通
- [ ] 房间→走廊→房间过渡流畅，无卡顿
- [ ] Boss出场动画2-3秒，有戏剧性
- [ ] 复活币系统正确：死亡→选择→消耗币复活或放弃
- [ ] 结算统计8项数据正确显示
- [ ] 评分S/A/B/C按加权公式正确计算
- [ ] 经验/金币按公式正确发放
- [ ] Performance: 副本流程无额外帧开销
- [ ] No hardcoded values in implementation

## Open Questions

| Question | Owner | Deadline | Resolution |
|----------|-------|----------|-----------|
| 副本是否有随机房间？ | game-designer | VS阶段 | MVP全固定，VS加入随机 |
| 是否有隐藏房间？ | game-designer | VS阶段 | MVP不做 |
| 每日副本/挑战模式？ | game-designer | Full Vision | 后期功能 |
