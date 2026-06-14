# Combat System

> **Status**: Draft
> **Author**: game-designer + gameplay-programmer
> **Last Updated**: 2026-06-15
> **Last Verified**: —
> **Implements Pillar**: 爽快打击+Q版反差 / 职业深度与成长 (战斗是两大支柱的交汇点)

## Summary

战斗系统定义了格斗萌主的核心战斗规则——攻击类型、动作取消、能量管理、防御机制和战斗状态流转。它是连招手感的骨架：Input System 提供输入，Player Controller 提供身体，Hit Detection 提供反馈，而 Combat System 决定"出什么招、能取消什么、消耗多少、怎么连"。

> **Quick reference** — Layer: `Feature` · Priority: `MVP` · Key deps: `Player Controller, Hit Detection & Feedback`

## Overview

Combat System 管理角色的战斗状态机（从待机到出招到收招到可取消）、三种攻击类型的行为差异、动作取消规则（什么动作可以打断什么）、能量系统（必杀技的资源）、防御/闪避机制、以及攻击的帧数据定义。它让"轻攻→轻攻→重攻→必杀"这条连招链不只是按键序列，而是有策略深度和节奏感的战斗体验。

## Player Fantasy

战斗应该像跳舞——轻攻是快步，重攻是重踏，必杀是旋转跳跃。每次出招都是承诺（你投入了时间和能量），每次取消都是救赎（你用更贵的动作换掉了当前动作）。闪避是紧急出口，但用多了就没有必杀能量。好玩家打得漂亮，菜玩家也能打得爽——3键连招就够了，但高手能挖掘出取消链的深度。

## Detailed Design

### Core Rules

1. **三种攻击类型**: 轻攻(快/低伤/低耗)、重攻(慢/中伤/中耗)、必杀(最慢/高伤/高耗能量)
2. **动作取消**: 特定动作可在recovery帧被更高优先级的动作取消
3. **能量系统**: 攻击命中积攒能量，必杀消耗能量，闪避消耗少量能量
4. **防御**: 无格挡机制（DNF式简化），闪避是唯一防御手段
5. **无敌帧**: 仅闪避和起身时有无敌帧
6. **空中战斗**: 可空中攻击，但只有1次空中攻击机会（着地重置）

### Attack Type Definitions

| Type | Startup | Active | Recovery | Damage | Energy Cost | Energy Gain(on hit) | Cancel Priority |
|------|---------|--------|----------|--------|-------------|---------------------|----------------|
| Light 1 | 2f | 3f | 4f | 10 | 0 | +5 | 1 |
| Light 2 | 2f | 3f | 4f | 12 | 0 | +5 | 1 |
| Light 3 | 3f | 4f | 5f | 15 | 0 | +8 | 1 |
| Heavy | 5f | 4f | 8f | 25 | 5 | +12 | 2 |
| Special | 8f | 6f | 12f | 50 | 30 | +0 | 3 |
| Air Light | 3f | 3f | 6f | 8 | 0 | +3 | 1 |
| Air Heavy | 6f | 4f | 10f | 20 | 5 | +8 | 2 |

### Cancel Rules

取消规则定义了哪些动作的recovery帧可以被哪些动作打断：

| Current Action | Can Cancel Into | Condition |
|----------------|----------------|-----------|
| Light 1 recovery | Light 2 / Heavy / Special / Dodge | 输入在cancel窗口内 |
| Light 2 recovery | Light 3 / Heavy / Special / Dodge | 输入在cancel窗口内 |
| Light 3 recovery | Heavy / Special / Dodge | 不可再接轻攻 |
| Heavy recovery | Special / Dodge | 不可取消回轻攻 |
| Special recovery | Dodge only | 必杀收招长，仅闪避可救 |
| Air any recovery | Nothing | 空中攻击不可取消 |
| Dodge recovery | Any attack | 闪避后可接任何攻击 |
| HitStun | Nothing | 硬直不可取消 |

**Cancel Window**: recovery帧的前50%是可取消窗口，后50%必须等动作自然结束

### Energy System

```
max_energy = 100
energy_gain_on_hit = attack_specific (see table above)
energy_gain_on_hurt = 3 (被击也少量获得)
energy_cost_dodge = 8
energy_cost_heavy = 5
energy_cost_special = 30
```

| Rule | Detail |
|------|--------|
| 初始能量 | 0 |
| 最大能量 | 100 |
| 必杀可用条件 | energy ≥ 30 |
| 闪避可用条件 | energy ≥ 8 |
| 重攻可用条件 | energy ≥ 5（或免费连招中） |
| 能量自然衰减 | 无——只增不减（除非使用） |
| 能量溢出 | 达到100后不再增加 |

### States and Transitions

| State | Entry Condition | Exit Condition | Can Move | Can Attack | Can Dodge |
|-------|----------------|----------------|----------|-----------|-----------|
| CombatIdle | 战斗中无动作 | 攻击/闪避/被击 | Yes | Yes | Yes |
| AttackStartup | 攻击输入 | 进入Active | No | No | No |
| AttackActive | Startup结束 | 进入Recovery | 攻击推力 | No | No |
| AttackRecovery | Active结束 | Cancel或自然结束 | 渐恢复 | Cancel only | Cancel only |
| DodgeActive | 闪避输入 | 位移完成 | 闪避速度 | No | No |
| HitStun | 被击中 | 硬直计时结束 | 击退减速 | No | No |
| Knockdown | 击倒(重击/必杀) | 起身动画完成 | No | No | 起身后3f无敌 |
| Knockup | 必杀击飞 | 着地 | No | No | No |

### Ground vs Air Combat

| Aspect | Ground | Air |
|--------|--------|-----|
| 可用攻击 | 全部 | Air Light / Air Heavy |
| 空中攻击次数 | — | 1次/跳跃 |
| 取消 | 按cancel规则 | 不可取消 |
| 移动 | 正常 | 攻击推力 |
| 着地 | — | 中断攻击→Idle |

### Interactions with Other Systems

| System | Interaction | Interface |
|--------|-------------|-----------|
| Input System | 消费攻击/闪避输入 | is_action_just_pressed(attack_*) |
| Player Controller | 驱动战斗状态 | set_combat_state(state) |
| Hit Detection | 命中时提供伤害参数 | on_hit_confirmed(type, damage) |
| Combo System | 连招链驱动攻击序列 | try_combo_advance(type) |
| Enemy AI | 敌人查询玩家战斗状态 | get_combat_state() |
| HUD | 显示能量条/技能状态 | signal: energy_changed(current, max) |

## Formulas

### Energy Gain

```
energy = clamp(current_energy + gain, 0, max_energy)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| current_energy | float | 0-100 | state | 当前能量值 |
| gain | float | 3-12 | config | 命中/被击获得的能量 |
| max_energy | int | 100 | config | 能量上限 |

### Cancel Window Calculation

```
cancel_window_end = recovery_start + floor(recovery_frames * cancel_ratio)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| cancel_window_end | int | 1-6 | calculated | 可取消窗口结束帧 |
| recovery_frames | int | 4-12 | attack data | recovery总帧数 |
| cancel_ratio | float | 0.5 | config | recovery前50%可取消 |

**Edge case**: recovery_frames = 4时，cancel_window = 2帧

### Damage with Combo Scaling

```
final_damage = floor(base_damage * (1.0 + combo_count * 0.1) * crit_multiplier)
```

(与 Hit Detection GDD 的公式一致，此处为Combat System的视角)

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|----------|------------------|-----------|
| 能量不足30按必杀 | 输入被忽略，无反馈 | 避免误导玩家以为"出了但没伤害" |
| 连招中必杀消耗所有能量 | 必杀释放，能量归0 | 必杀后需要重新积攒 |
| 空中被击中 | 进入空中HitStun，着地后额外10帧硬直 | 空中受击更危险 |
| 必杀收招中被击 | 正常受击，无特殊保护 | 必杀的高风险：长收招 |
| 连招中按闪避但能量不足 | 闪避不触发，连招继续 | 能量管理是策略核心 |
| Dodge后立即攻击 | Dodge recovery帧内可取消→攻击 | 闪避反击是核心玩法 |
| 同时按轻攻+重攻 | 重攻优先(Input System已定义) | 避免误触 |
| 被击倒起身 | 3帧无敌窗口 | 防止起身再次被击(wake-up pressure) |

## Dependencies

| System | Direction | Nature of Dependency |
|--------|-----------|---------------------|
| Input System | This depends on Input | 消费攻击/闪避输入 |
| Player Controller | Bidirectional | 驱动战斗状态/查询身体状态 |
| Hit Detection | Bidirectional | 提供攻击参数/接收命中结果 |
| Combo System | Combo depends on this | 连招链基于战斗状态 |
| Enemy AI | Enemy queries this | 判断玩家可攻击窗口 |
| HUD | HUD depends on this | 显示能量/技能冷却 |

## Tuning Knobs

| Parameter | Current Value | Safe Range | Effect of Increase | Effect of Decrease |
|-----------|--------------|------------|-------------------|-------------------|
| max_energy | 100 | 50-200 | 必杀更难攒满 | 必杀更频繁 |
| special_cost | 30 | 20-60 | 必杀更贵更稀有 | 必杀更便宜更频繁 |
| dodge_cost | 8 | 0-20 | 闪避更贵 | 闪避几乎免费 |
| heavy_cost | 5 | 0-15 | 重攻有代价 | 重攻免费 |
| cancel_ratio | 0.5 | 0.3-0.8 | 取消窗口更宽 | 取消窗口更窄 |
| energy_per_light_hit | 5 | 2-10 | 轻攻积攒更快 | 轻攻积攒更慢 |
| energy_per_heavy_hit | 12 | 5-20 | 重攻积攒更快 | 重攻积攒更慢 |
| i_frames_on_wake | 3 | 0-10 | 起身更安全 | 起身更危险 |
| air_attack_count | 1 | 0-3 | 空中战斗更丰富 | 纯地面战斗 |
| knockdown_threshold | HP≤0 | 触发条件 | 何时击倒 | 控制击倒频率 |

## Visual/Audio Requirements

| Event | Visual Feedback | Audio Feedback | Priority |
|-------|----------------|---------------|----------|
| 能量满 | 角色边缘发光脉冲 | 能量充满提示音 | Medium |
| 必杀可用 | 技能图标高亮闪烁 | 无 | High |
| 能量不足 | 技能图标暗灰+按无效 | 低沉"不可"音效 | Medium |
| 必杀释放 | 全屏闪白+角色蓄力特效 | 必杀蓄力音→爆发音 | Critical |
| 闪避 | 半透明残影+拖影 | 闪避风声 | High |
| 击倒 | 角色倒地动画+灰尘 | 击倒重响 | High |
| 起身 | 起身动画+3帧无敌闪光 | 起身音效 | Medium |

## Game Feel

### Feel Reference
应像《苍翼默示录》的战斗节奏——轻攻可以快速连按确认，重攻需要择机投入，必杀是改变战局的大招。取消链让高手能打出远超基础连招的精彩操作，但3键基础连招已经够爽。**不应像**DNF的深度搓招——那让新手望而却步。

### Input Responsiveness

| Action | Max Input-to-Response Latency (ms) | Frame Budget | Notes |
|--------|-----------------------------------|-------------|-------|
| Light attack | 33ms | 2f startup | 即时出招 |
| Heavy attack | 83ms | 5f startup | 有蓄力感 |
| Special | 133ms | 8f startup | 明显蓄力 |
| Dodge | 16ms | 1f startup | 紧急逃脱 |
| Cancel | 33ms | 2f | 取消必须快 |

### Animation Feel Targets

| Animation | Startup | Active | Recovery | Cancel Window | Feel Goal |
|-----------|---------|--------|----------|--------------|-----------|
| Light 1 | 2f | 3f | 4f | 2f | 极快，低承诺 |
| Light 2 | 2f | 3f | 4f | 2f | 同上 |
| Light 3 | 3f | 4f | 5f | 2f | 收尾击，略重 |
| Heavy | 5f | 4f | 8f | 4f | 有分量，可取消 |
| Special | 8f | 6f | 12f | 6f | 史诗，高风险 |
| Dodge | 1f | 8f | 4f | — | 即时逃脱 |
| Air Light | 3f | 3f | 6f | 0 | 不可取消 |
| Air Heavy | 6f | 4f | 10f | 0 | 不可取消 |

### Impact Moments

| Impact Type | Duration (ms) | Effect Description | Configurable? |
|-------------|--------------|-------------------|---------------|
| 必杀蓄力闪光 | 133ms | 角色发光+周围粒子吸引 | Yes |
| 必杀释放冲击波 | 200ms | 圆形冲击波扩散 | Yes |
| 击倒着地 | 100ms | 屏幕微震+灰尘爆发 | Yes |
| 能量充满瞬间 | 67ms | 角色轮廓金色脉冲 | Yes |

### Weight and Responsiveness Profile

- **Weight**: 轻攻=蜜蜂蜇(快而轻)，重攻=锤子砸(慢而沉)，必杀=核弹(最慢最重)
- **Player control**: 轻攻极高(可随时取消)，重攻中等(只能取消到必杀/闪避)，必杀低(只有闪避可救)
- **Snap quality**: 所有攻击startup是锐利的二元切换，不是渐变
- **Acceleration model**: 攻击推力在Active帧开始时即时施加
- **Failure texture**: 必杀收招被打→明确的"我贪了"感；能量不足闪避不了→"我浪费了"感

### Feel Acceptance Criteria

- [ ] 轻攻2帧内可见角色动作启动
- [ ] J-J-K-L 连招在3秒内可完成，节奏感明显
- [ ] 必杀释放有明显蓄力→爆发的戏剧性
- [ ] 闪避可100%打断除必杀recovery后段外的所有动作
- [ ] 能量管理让玩家在"存必杀"和"用闪避"间做选择
- [ ] 无测试者说"我按了但没反应"（除非能量不足）

## UI Requirements

| Information | Display Location | Update Frequency | Condition |
|-------------|-----------------|-----------------|-----------|
| 能量条 | HP条下方 | 每帧 | 战斗中 |
| 必杀可用指示 | 能量条旁图标 | 能量≥30时 | 持续 |
| 技能冷却 | 技能栏图标 | 每帧 | 冷却中 |
| 连招路径提示(新手) | 画面底部 | 连招进行中 | 教学模式 |

## Cross-References

| This Document References | Target GDD | Specific Element Referenced | Nature |
|--------------------------|-----------|----------------------------|--------|
| 攻击输入来源 | `design/gdd/input-system.md` | attack_light/heavy/special | Data dependency |
| 角色战斗状态 | `design/gdd/player-controller.md` | combat_state枚举 | State trigger |
| 命中反馈触发 | `design/gdd/hit-detection-feedback.md` | hit_confirmed signal | State trigger |
| 连招取消规则 | `design/gdd/combo-system.md` | combo_chain定义 | Rule dependency |
| 必杀特效样式 | `design/art/art-bible.md` | VFX Standards | Rule dependency |

## Acceptance Criteria

- [ ] 3种攻击类型的startup/active/recovery帧数据与设计表一致
- [ ] Cancel规则在所有组合下正确工作（轻→轻→重→必杀链路畅通）
- [ ] 能量系统：命中+5/+12，闪避-8，重攻-5，必杀-30
- [ ] 必杀在能量<30时不可释放
- [ ] 闪避可打断轻攻/重攻/必杀recovery
- [ ] 空中攻击1次/跳跃，着地重置
- [ ] 被击倒起身3帧无敌
- [ ] Performance: 战斗状态机update < 0.5ms
- [ ] No hardcoded values in implementation

## Open Questions

| Question | Owner | Deadline | Resolution |
|----------|-------|----------|-----------|
| 重攻是否免费？还是消耗能量？ | game-designer | GDD审批时 | 当前设计：消耗5能量 |
| 是否需要"破防"机制？ | game-designer | VS阶段 | MVP不做 |
| 空中必杀？ | game-designer | GDD审批时 | MVP不做，空中只有轻/重攻 |
