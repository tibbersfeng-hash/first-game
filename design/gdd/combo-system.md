# Combo System

> **Status**: Draft
> **Author**: game-designer
> **Last Updated**: 2026-06-15
> **Last Verified**: —
> **Implements Pillar**: 爽快打击+Q版反差 / 职业深度与成长 (连招是两大支柱的操作体现)

## Summary

连招系统管理连招链的定义、输入窗口、连招计数和连招类型路由。它将玩家的按键序列解析为结构化的连招链，而非零散的攻击。核心目标是让3-4键的简单连招"傻瓜式爽"，同时为高手提供取消链的深度探索空间。

> **Quick reference** — Layer: `Feature` · Priority: `MVP` · Key deps: `Combat System`

## Overview

Combo System 是"按键序列→连招体验"的翻译器。玩家按J-J-K-L，本系统识别为"基础连招"并逐步推进连招链、累加连击数、调整输入窗口。它不决定出什么招（那是Combat System的事），但决定"这几个招能不能连起来"以及"连得紧不紧"。

## Player Fantasy

按J-J-K-L时感觉像在弹钢琴——每个键都有回响，连在一起是一首小曲。按错一个键？没关系，系统自动帮你找到最接近的连招链。高手会发现隐藏的取消路径，菜鸟只会J-J-J也能打出收尾重击。连击数不断攀升时，画面反馈越来越炸。

## Detailed Design

### Core Rules

1. **连招链预定义**: 每个连招链是一个有序的攻击类型序列，在配置文件中定义
2. **输入窗口递减**: 连招越深，下一步输入窗口越短（衰减公式）
3. **容错匹配**: 错误输入不会立即中断连招，尝试匹配其他链
4. **连击计数**: 每次成功命中+1，超时/被击/换目标重置
5. **连招超时**: 无输入超过阈值时连招自动结束
6. **视觉升级**: 连击数越高，反馈越强（屏幕色差/额外粒子/数字缩放）

### Combo Chain Definitions

| Chain Name | Input Sequence | Total Hits | Total Damage | Difficulty | Notes |
|------------|---------------|------------|-------------|-----------|-------|
| 基础连 | L→L→H→S | 4 | 97 (10+12+25+50) | ★☆☆ | 入门必备 |
| 速连 | L→L→L | 3 | 37 (10+12+15) | ★☆☆ | 快速收尾 |
| 力连 | H→S | 2 | 75 (25+50) | ★★☆ | 爆发伤害 |
| 闪反连 | Dodge→L→H | 3 | 35 (0+10+25) | ★★☆ | 闪避反击 |
| 空连 | AirL→AirH | 2 | 28 (8+20) | ★★☆ | 空中压制 |

(L=Light, H=Heavy, S=Special, AirL=AirLight, AirH=AirHeavy)

### Input Window Decay

```
window_ms = base_window * (1 - step_index * decay_rate)
window_ms = max(window_ms, min_window)
```

| Chain Step | base_window | decay_rate | Result Window | Notes |
|-----------|------------|------------|--------------|-------|
| Step 1 (L→L) | 300ms | 0.10 | 270ms | 很宽容 |
| Step 2 (L→H) | 300ms | 0.10 | 240ms | 仍宽容 |
| Step 3 (H→S) | 300ms | 0.10 | 210ms | 需要一点节奏 |
| Step 4+ | 300ms | 0.10 | 180ms→150ms | 逐渐收紧 |

### Combo Count & Scaling

| Combo Count | Damage Bonus | Visual Effect | Audio Effect |
|------------|-------------|---------------|-------------|
| 1-4 | +0% ~ +40% | 无特殊 | 正常音效 |
| 5-9 | +50% ~ +90% | 轻微屏幕色差 | 音效微升调 |
| 10-14 | +100% ~ +140% | 明显色差+额外粒子 | 音效升调+回响 |
| 15-19 | +150% ~ +190% | 强色差+屏幕边缘发光 | 音效强升调 |
| 20+ | +200% (cap) | 全屏脉冲+金光 | 最高音效+回响 |

### Fallback Matching

当玩家输入了"错误"的下一步时，系统不立即丢弃连招：

1. 在所有链中搜索以当前输入开头的链 → 找到则切换链
2. 搜索当前链是否有跳步路径（如跳过一步仍可继续）→ 找到则跳步
3. 都找不到 → 连招中断，该输入作为新连招的第一步

```
示例: 基础连(L→L→H→S)执行到Step2时按了L(而非H)
  → 搜索以L开头的链: 速连(L→L→L)匹配
  → 切换到速连，从Step2继续
  → 连击数保留
```

### Combo Timeout Rules

| Condition | Timeout | Behavior |
|-----------|---------|----------|
| 连招中间步骤 | 当前步骤window_ms | 超时→连招中断 |
| 连招完成后（无新输入） | 800ms | 超时→连击数清零 |
| 被击中 | 立即 | 连击数清零 |
| 换目标 | 保留 | 连击数跨目标保留 |
| 着地（空中连招） | 500ms | 短暂延伸窗口 |

### States and Transitions

| State | Entry Condition | Exit Condition | Behavior |
|-------|----------------|----------------|----------|
| Idle | 无连招 | 攻击输入 | 不跟踪连击 |
| ChainActive | 连招第一步命中 | 完成/超时/中断 | 跟踪连击数+窗口 |
| ChainComplete | 连招链最后一步完成 | 超时/新输入 | 保留连击数，等待后续 |
| ChainBroken | 超时/被击 | 新攻击输入 | 连击数清零，新连招开始 |

### Interactions with Other Systems

| System | Interaction | Interface |
|--------|-------------|-----------|
| Combat System | 连招驱动攻击序列 | try_advance(attack_type) → bool |
| Input System | 缓冲窗口受连招状态影响 | query: get_current_window() |
| Hit Detection | 命中确认推进连击数 | on_hit_confirmed() → combo_count++ |
| HUD | 显示连击数/连招名 | signal: combo_updated(count, chain_name) |
| Audio | 连击数影响音效 | signal: combo_count_changed(count) |

## Formulas

### Input Window Calculation

```
window = max(base_window * (1 - step * decay), min_window)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| window | float | 0.1-0.3 | calculated | 输入窗口(秒) |
| base_window | float | 0.2-0.4 | config | 基础窗口 |
| step | int | 0-10 | state | 当前连招步骤(0-indexed) |
| decay | float | 0.05-0.15 | config | 每步衰减率 |
| min_window | float | 0.1-0.15 | config | 最低窗口保证 |

**Expected output range**: 300ms (step 0) → 150ms (step 10+)

### Combo Damage Bonus

```
bonus_multiplier = 1.0 + combo_count * combo_bonus_rate
bonus_multiplier = min(bonus_multiplier, max_bonus)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| bonus_multiplier | float | 1.0-3.0 | calculated | 伤害倍率 |
| combo_count | int | 0-20 | state | 当前连击数 |
| combo_bonus_rate | float | 0.08-0.12 | config | 每击加成率 |
| max_bonus | float | 3.0 | config | 最高3倍 |

### Combo Score (for HUD display)

```
score = sum(hit_damage * combo_multiplier) * chain_completion_bonus
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| chain_completion_bonus | float | 1.0-1.5 | config | 完成整条链的奖励倍率 |

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|----------|------------------|-----------|
| 连招中间按了完全不相关的键 | Fallback匹配→无匹配→中断，该键作为新连招第一步 | 不惩罚探索 |
| 连招中间被击中 | 连击数清零，进入HitStun | 被打断的代价 |
| 两条链共享前缀(L→L) | 输入L→L后按H走基础连，按L走速连 | 自动分流 |
| 空中连招着地 | 500ms延伸窗口，可继续地面连招 | 奖励空中→地面衔接 |
| 连招中必杀能量不足 | 连招中断在必杀步骤，不消耗能量 | 不能释放就不释放 |
| 极快连按(50ms内两次轻攻) | 消抖机制忽略第二次 | 防止连发 |
| 目标死亡但连招未完 | 连击数保留，可继续攻击新目标 | 奖励清场效率 |

## Dependencies

| System | Direction | Nature of Dependency |
|--------|-----------|---------------------|
| Combat System | This depends on Combat | 攻击类型和取消规则由Combat定义 |
| Input System | Input depends on this | 缓冲窗口受连招步骤影响 |
| Hit Detection | This depends on Hit Detection | 命中确认推进连击数 |
| HUD | HUD depends on this | 显示连击数和视觉升级 |

## Tuning Knobs

| Parameter | Current Value | Safe Range | Effect of Increase | Effect of Decrease |
|-----------|--------------|------------|-------------------|-------------------|
| base_window | 300ms | 150-500ms | 连招更宽容 | 连招更严格 |
| decay_rate | 0.10 | 0.05-0.20 | 深层连招更紧 | 深层连招仍宽 |
| min_window | 150ms | 80-200ms | 最深连招保证 | 最深连招极严格 |
| combo_timeout | 800ms | 400-1500ms | 连击保留更久 | 连击快速清零 |
| combo_bonus_rate | 0.10 | 0.05-0.20 | 连击伤害增长更快 | 连击奖励更低 |
| max_bonus | 3.0 | 2.0-5.0 | 高连击上限更高 | 高连击上限更低 |
| chain_completion_bonus | 1.5 | 1.0-2.0 | 完成整条链奖励更大 | 完成奖励更小 |

## Visual/Audio Requirements

| Event | Visual Feedback | Audio Feedback | Priority |
|-------|----------------|---------------|----------|
| 连击5+ | 轻微色差 | 音效微升 | Medium |
| 连击10+ | 明显色差+额外粒子 | 音效升调 | High |
| 连击15+ | 强色差+边缘光 | 音效强升调 | High |
| 连击20+ | 全屏脉冲+金光 | 最高音效 | Critical |
| 连招中断 | COMBO数字闪红消失 | 下坠音效 | Medium |
| 连招完成 | 连招名闪现+金色脉冲 | 完成音效 | High |

## Game Feel

### Feel Reference
应像《鬼泣》的连招系统——按键序列变成华丽表演，连击数是鼓励你继续的动力。但门槛更低——3键就够了，不需要6键搓招。**不应像**《街头霸王》的严格输入——那需要肌肉记忆，我们的目标是有节奏感就行。

### Feel Acceptance Criteria

- [ ] J-J-K-L 基础连在3秒内可完成，节奏感明显
- [ ] 新手只按J-J-J也能完成速连，不会"卡住"
- [ ] 连击10+时视觉反馈明显升级，测试者注意到变化
- [ ] 按错键不会让连招"莫名其妙断了"，而是平滑过渡
- [ ] 连击数字大到让人想截图（20+有特殊视觉）

## UI Requirements

| Information | Display Location | Update Frequency | Condition |
|-------------|-----------------|-----------------|-----------|
| 连击数 | 屏幕中上方 | 每次命中 | combo > 0 |
| 连招名 | 连击数下方 | 连招完成时 | 短暂显示1.5s |
| 输入窗口指示 | 技能栏微弱高亮 | 每帧 | 连招进行中 |

## Cross-References

| This Document References | Target GDD | Specific Element Referenced | Nature |
|--------------------------|-----------|----------------------------|--------|
| 攻击类型定义 | `design/gdd/combat-system.md` | Light/Heavy/Special帧数据 | Rule dependency |
| 输入窗口影响缓冲 | `design/gdd/input-system.md` | buffer_duration | Data dependency |
| 连击数影响伤害 | `design/gdd/hit-detection-feedback.md` | combo_multiplier | Data dependency |
| 连击视觉反馈 | `design/art/art-bible.md` | VFX Standards | Rule dependency |

## Acceptance Criteria

- [ ] 5条连招链全部可通过正确输入序列触发
- [ ] Fallback匹配：错误输入时50%以上情况能找到替代链
- [ ] 输入窗口按衰减公式正确递减
- [ ] 连击数跨目标保留，被击/超时清零
- [ ] 连击10+时触发色差视觉升级
- [ ] 连击20+触发全屏脉冲+3x伤害上限
- [ ] Performance: 连招匹配在1帧内完成
- [ ] No hardcoded values in implementation

## Open Questions

| Question | Owner | Deadline | Resolution |
|----------|-------|----------|-----------|
| 取消连招（如跳跃取消）？ | game-designer | VS阶段 | MVP不做，深层连招够用 |
| 隐藏连招（特殊输入序列）？ | game-designer | VS阶段 | 有趣但MVP不需要 |
| 连招评分（S/A/B/C）？ | game-designer | Dungeon Flow GDD时 | 与副本评分结合 |
