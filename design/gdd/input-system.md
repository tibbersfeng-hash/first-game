# Input System

> **Status**: Draft
> **Author**: game-designer
> **Last Updated**: 2026-06-15
> **Last Verified**: —
> **Implements Pillar**: 爽快打击+Q版反差 (输入响应是打击感的基石)

## Summary

输入系统管理键盘和手柄的所有玩家输入映射、输入缓冲、优先级裁决和上下文切换。它是战斗手感的基石——延迟1帧就是手感的生死之差。本系统为所有上层系统提供统一的输入事件接口。

> **Quick reference** — Layer: `Foundation` · Priority: `MVP` · Key deps: `None`

## Overview

格斗萌主的输入系统负责将物理输入（键盘按键/手柄摇杆按钮）转换为游戏动作事件。它提供输入缓冲窗口让连招输入更宽容、上下文感知让同一按键在不同场景有不同行为、优先级裁决避免动作冲突。目标是让玩家感觉"按键就出招"，而非"按键后等系统响应"。

## Player Fantasy

玩家按下攻击键的瞬间角色就应该动起来——零延迟、零犹豫。连招输入可以在前一招还没结束时提前输入，系统记住意图在正确时机自动衔接。手柄和键盘体验完全对等，不存在"必须用哪个"的强迫感。

## Detailed Design

### Core Rules

1. **输入映射**: 每个游戏动作（move_left, jump, attack_light等）映射到一个或多个物理输入，通过 Godot InputMap 配置
2. **输入缓冲**: 攻击/跳跃/闪避等动作有输入缓冲窗口（buffer_window），在缓冲窗口内的输入会在当前动作结束后自动触发
3. **上下文切换**: 同一输入在不同状态下可映射到不同动作（如空中攻击vs地面攻击），由 Player Controller 的状态机决定
4. **优先级裁决**: 当多个输入同时到达时，按优先级处理：闪避 > 攻击 > 移动
5. **输入消抖**: 同一输入在 action_pressed 后的 100ms 内不重复触发
6. **手柄支持**: 左摇杆移动、A/B/X/Y对应跳跃/轻攻/重攻/特殊，完全对等键盘

### Input Map Definition

| Action | Keyboard | Gamepad | Context |
|--------|----------|---------|---------|
| move_left | A / ← | 左摇杆左 | 全局 |
| move_right | D / → | 左摇杆右 | 全局 |
| move_up | W / ↑ | 左摇杆上 | 梯子/菜单 |
| move_down | S / ↓ | 左摇杆下 | 梯子/菜单 |
| jump | W / Space | A (底部) | 地面/空中 |
| attack_light | J | X (左侧) | 战斗 |
| attack_heavy | K | Y (顶部) | 战斗 |
| attack_special | L | B (右侧) | 战斗 |
| dodge | Shift | RB | 战斗 |
| interact | E | LB | 非战斗 |
| pause | Esc | Start | 全局 |

### States and Transitions

| State | Entry Condition | Exit Condition | Behavior |
|-------|----------------|----------------|----------|
| Normal | 默认 | 进入任何受限状态 | 所有输入正常响应 |
| InCombo | 攻击触发连招 | 连招结束/超时/被打断 | 缓冲窗口激活，只接受攻击/闪避输入 |
| InHitStun | 被击中 | 硬直结束 | 输入被抑制，仅缓冲第一个逃脱输入 |
| InCutscene | 过场动画开始 | 过场结束 | 只响应pause输入 |
| MenuOpen | 菜单打开 | 菜单关闭 | 输入路由到UI系统 |

### Input Buffer Rules

1. 缓冲队列最大容量：2个动作
2. 缓冲窗口：轻攻200ms，重攻150ms，必杀100ms，跳跃300ms，闪避250ms
3. 缓冲输入在触发后被消费（清出队列）
4. 如果缓冲的动作在窗口结束前仍未被消费，丢弃
5. 新缓冲输入覆盖同类型旧输入（最后意图优先）

### Interactions with Other Systems

| System | Interaction | Interface |
|--------|-------------|-----------|
| Player Controller | 消费输入事件驱动角色状态 | InputSystem.get_buffered_action() |
| Combat System | 攻击输入触发战斗状态 | signal: attack_requested(type) |
| Combo System | 连招期间缓冲窗口控制 | query: is_in_combo() |
| HUD System | 显示输入相关UI（技能冷却/缓冲提示） | signal: action_buffered(action) |
| UI System | 菜单状态切换输入路由 | signal: input_context_changed(context) |

## Formulas

### Buffer Window Duration

```
buffer_duration = base_buffer * (1 - combo_step * combo_decay)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| buffer_duration | float | 0.05-0.3 | calculated | 实际缓冲窗口时长(秒) |
| base_buffer | float | 0.1-0.3 | config | 基础缓冲窗口 |
| combo_step | int | 0-4 | combo system | 当前连招第几步 |
| combo_decay | float | 0.0-0.15 | config | 每步连招缓冲衰减 |

**Expected output range**: 0.05s (连招末端) to 0.3s (初始)
**Edge case**: combo_step > 4 时，buffer_duration 不低于 0.05s（最低保证）

### Input Priority Score

```
priority = base_priority * urgency_modifier
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| priority | float | 1-100 | calculated | 输入优先级分数 |
| base_priority | float | 10-80 | config | 动作基础优先级 |
| urgency_modifier | float | 0.5-2.0 | config | 紧急度修正（被击中时闪避×2） |

**Priority table**: dodge=80, attack_special=60, attack_heavy=50, attack_light=40, jump=30, move=10

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|----------|------------------|-----------|
| 同时按下轻攻和重攻 | 重攻优先（优先级高） | 避免误触发轻攻 |
| 连招中按下闪避 | 立即中断连招执行闪避 | 闪避是最高优先级逃脱手段 |
| 空中按跳跃 | 不触发二段跳（除非角色有该能力） | 防止无限跳跃 |
| 缓冲队列已满时再按攻击 | 覆盖最早的缓冲动作 | 最新意图优先 |
| 手柄断连 | 暂停游戏+提示重连 | 防止角色失控 |
| 按键持续按住 | 只触发一次pressed，held用于移动 | 避免攻击连发 |

## Dependencies

| System | Direction | Nature of Dependency |
|--------|-----------|---------------------|
| Player Controller | Player Controller depends on this | 消费输入事件 |
| Combat System | Combat System depends on this | 接收攻击请求信号 |
| Combo System | Combo System depends on this | 查询连招状态影响缓冲 |
| HUD System | HUD System depends on this | 显示技能/缓冲状态 |

## Tuning Knobs

| Parameter | Current Value | Safe Range | Effect of Increase | Effect of Decrease |
|-----------|--------------|------------|-------------------|-------------------|
| base_buffer (轻攻) | 200ms | 100-400ms | 连招更容易，手感更宽容 | 连招更严格，需精确输入 |
| base_buffer (重攻) | 150ms | 80-300ms | 重攻更容易衔接 | 重攻需更精准 |
| base_buffer (必杀) | 100ms | 50-200ms | 必杀释放更宽松 | 必杀需精确时机 |
| combo_decay | 0.10 | 0.0-0.15 | 连招后期缓冲更短 | 连招后期缓冲保持 |
| min_buffer | 50ms | 30-100ms | 最低保证更宽 | 连招末端极严格 |
| dodge_priority | 80 | 50-100 | 闪避更容易打断攻击 | 闪避可能被攻击覆盖 |

## Visual/Audio Requirements

| Event | Visual Feedback | Audio Feedback | Priority |
|-------|----------------|---------------|----------|
| 攻击输入被缓冲 | 微弱的按键图标脉冲 | 无 | Low |
| 缓冲动作成功触发 | 技能图标短暂高亮 | 无 | Low |
| 闪避打断连招 | 角色闪烁 + 残影 | 闪避音效 | Medium |
| 输入被抑制(硬直中) | 微弱红色闪烁 | 低沉提示音 | Low |

## Game Feel

### Feel Reference
应像《街头霸王》的输入系统——零延迟响应+宽容缓冲窗口。输入缓冲让"提前输入"成为自然行为，而非需要精确计时。**不应像**早期魂系的严格输入窗口——那让玩家感到"我明明按了为什么不响应"。

### Input Responsiveness

| Action | Max Input-to-Response Latency (ms) | Frame Budget (at 60fps) | Notes |
|--------|-----------------------------------|------------------------|-------|
| 轻攻 | 33ms | 2帧 | 必须极速，连招基础 |
| 重攻 | 50ms | 3帧 | 可稍慢，蓄力感 |
| 必杀 | 66ms | 4帧 | 蓄力动画允许更多延迟 |
| 跳跃 | 33ms | 2帧 | 平台游戏基本要求 |
| 闪避 | 33ms | 2帧 | 逃脱动作必须即时 |
| 移动 | 16ms | 1帧 | 最优先，持续输入 |

### Animation Feel Targets

| Animation | Startup Frames | Active Frames | Recovery Frames | Feel Goal | Notes |
|-----------|---------------|--------------|----------------|-----------|-------|
| 轻攻1 | 2 | 3 | 4 | 极速，低承诺 | 可被后续连招取消 |
| 轻攻2 | 2 | 3 | 4 | 同上 | |
| 重攻 | 5 | 4 | 8 | 有分量，高承诺 | 不可轻易取消 |
| 必杀 | 8 | 6 | 12 | 史诗感，最高承诺 | 蓄力→爆发→长收招 |
| 闪避 | 1 | 8 | 4 | 即时逃脱 | 启动帧最短 |
| 跳跃 | 1 | — | — | 即时离地 | 无启动帧 |

### Impact Moments

| Impact Type | Duration (ms) | Effect Description | Configurable? |
|-------------|--------------|-------------------|---------------|
| Hit-stop (轻攻命中) | 50ms | 3帧冻结 | Yes |
| Hit-stop (重攻命中) | 133ms | 8帧冻结 | Yes |
| Hit-stop (必杀命中) | 200ms | 12帧冻结 | Yes |
| 屏幕震动 (必杀释放) | 150ms | 方向性震动，衰减 | Yes |

### Weight and Responsiveness Profile

- **Weight**: 轻攻=轻快如蜂，重攻=沉如有锤。输入响应全部轻快，但动画commitment分级
- **Player control**: 轻攻高控制（可随时取消），重攻中等（仅闪避可取消），必杀低控制（全力投入）
- **Snap quality**: 所有输入响应是即时的二元触发，没有渐变加速
- **Acceleration model**: 移动是即时的arcade feel（无加速曲线），攻击也是即时触发
- **Failure texture**: 输入被硬直抑制时，角色有明显的受击状态视觉，玩家能清晰理解"为什么没响应"

### Feel Acceptance Criteria

- [ ] 玩家从按下攻击键到角色动画启动不超过2帧
- [ ] 连招输入可以在前一招结束前300ms提前输入并成功衔接
- [ ] 闪避能100%打断除必杀收招外的所有动作
- [ ] 手柄和键盘体验完全一致，无功能缺失
- [ ] 无测试者反馈"我按了但没反应"

## UI Requirements

| Information | Display Location | Update Frequency | Condition |
|-------------|-----------------|-----------------|-----------|
| 技能图标 | HUD右下角 | 每帧 | 持续显示 |
| 技能冷却 | 技能图标上叠加 | 每帧 | 冷却中 |
| 缓冲输入指示 | 技能图标微弱高亮 | 输入时 | 缓冲中有输入 |
| 输入提示(教学) | 画面中央下方 | 事件触发 | 新手引导 |

## Cross-References

| This Document References | Target GDD | Specific Element Referenced | Nature |
|--------------------------|-----------|----------------------------|--------|
| combo_step影响缓冲窗口 | `design/gdd/combo-system.md` | combo_step值 | Data dependency |
| 闪避打断连招 | `design/gdd/combat-system.md` | 动作取消规则 | Rule dependency |
| 手柄按键映射 | `.claude/docs/technical-preferences.md` | Input Methods配置 | Rule dependency |

## Acceptance Criteria

- [ ] 所有11个动作输入在键盘和手柄上均可触发，无遗漏
- [ ] 输入缓冲窗口在config中可配置，无硬编码
- [ ] 缓冲队列正确消费和丢弃，不会积累幽灵输入
- [ ] 闪避优先级高于所有攻击，可可靠打断
- [ ] 上下文切换在Normal/InCombo/InHitStun/MenuOpen间正确转换
- [ ] Performance: 输入轮询在_physics_process中完成，耗时<0.5ms
- [ ] No hardcoded values in implementation

## Open Questions

| Question | Owner | Deadline | Resolution |
|----------|-------|----------|-----------|
| 是否需要宏按键支持？ | game-designer | GDD审批时 | 倾向不做，防止连发外挂 |
| 手柄震动反馈强度？ | game-designer | Audio System GDD时 | 与Audio系统一起定义 |
