# ADR-003: Signal Bus over Direct References

## Status
Accepted

## Date
2026-06-15

## Engine Compatibility

| Field | Value |
|-------|-------|
| **Engine** | Godot 4.6.3 |
| **Domain** | Core |
| **Knowledge Risk** | HIGH — Godot 4.6 is post-LLM-cutoff |
| **References Consulted** | `docs/engine-reference/godot/VERSION.md`, `docs/engine-reference/godot/breaking-changes.md`, `docs/engine-reference/godot/deprecated-apis.md` |
| **Post-Cutoff APIs Used** | None — Godot Signal system is stable since 4.0 |
| **Verification Required** | Confirm typed signal signatures compile in GDScript 2.0; verify no signal-name collisions across systems |

## ADR Dependencies

| Field | Value |
|-------|-------|
| **Depends On** | ADR-002 (Autoload Singletons — SignalBus is itself an Autoload) |
| **Enables** | All cross-system communication (Combat→HUD, Enemy→DungeonFlow, Player→HUD) |
| **Blocks** | HUD System GDD implementation (needs combo_updated, hit_confirmed signals) |
| **Ordering Note** | Must be decided before any gameplay code that crosses system boundaries |

## Context

### Problem Statement
格斗萌主的 9 个 MVP 系统之间需要通信：Combat System 需要通知 HUD 显示连击数，Hit Detection 需要通知 Damage Number 显示伤害，Enemy AI 需要通知 Dungeon Room 自己已死亡。如何实现这些跨系统通信，既保持解耦又不牺牲性能？

### Constraints
- 格斗游戏的战斗反馈必须 < 1帧延迟（16.67ms 内），信号分发不能成为瓶颈
- HUD 需要监听多个系统的输出（HP 变化、连击更新、能量变化、房间切换）
- 系统间通信必须单向——HUD 不应反向调用 Combat System
- 调试时需要能追踪"谁发了什么信号、谁收到了"

### Requirements
- 跨层系统间通信必须解耦——发射方不知道接收方存在
- 信号分发延迟 < 0.1ms（在 _physics_process 内同步分发）
- 新增监听者不需要修改发射方代码
- 信号名必须有命名规范，避免冲突

## Decision

使用 **SignalBus Autoload** 作为跨层通信的唯一机制。SignalBus 是一个只声明信号的 Godot Autoload 单例，不持有任何状态。

### 架构图

```
┌──────────────┐     emit     ┌────────────┐    receive    ┌──────────┐
│ CombatSystem │─────────────▶│  SignalBus │─────────────▶│   HUD    │
│              │              │ (Autoload) │              │          │
│ EnemyAI      │─────────────▶│            │─────────────▶│DngnFlow  │
│ PlayerCtrl   │─────────────▶│            │─────────────▶│AudioMgr  │
│ HitDetection │─────────────▶│            │─────────────▶│DmgNumber │
└──────────────┘              └────────────┘              └──────────┘

                    ↑ No state, only signals ↑
```

### 信号清单（MVP 范围）

```gdscript
# SignalBus.gd — Autoload
extends Node

# ─── Player 状态 ───
signal player_health_changed(current: int, maximum: int)
signal player_energy_changed(current: float, maximum: float)
signal player_died

# ─── 战斗事件 ───
signal hit_confirmed(target: Node2D, damage: int, damage_type: int, is_crit: bool)
signal damage_number_requested(position: Vector2, damage: int, damage_type: int, is_crit: bool)
signal attack_started(attack_id: String)
signal attack_hit(attack_id: String, target: Node2D)

# ─── 连招系统 ───
signal combo_updated(count: int, chain_name: String)
signal combo_dropped
signal combo_maxed

# ─── 敌人事件 ───
signal enemy_died(enemy: CharacterBody2D)
signal enemy_hit(enemy: CharacterBody2D, damage: int)

# ─── 副本流程 ───
signal room_entered(room_id: String)
signal room_cleared(rating: String)
signal dungeon_cleared(dungeon_id: String, stats: Dictionary)
signal dungeon_failed(reason: String)

# ─── 游戏状态 ───
signal game_paused
signal game_resumed
signal game_over
```

### 信号命名规范

| 前缀 | 含义 | 示例 |
|------|------|------|
| `player_` | 玩家状态变更 | `player_health_changed` |
| `combo_` | 连招系统事件 | `combo_updated` |
| `enemy_` | 敌人事件 | `enemy_died` |
| `room_` / `dungeon_` | 副本流程事件 | `room_cleared` |
| `hit_` / `attack_` / `damage_` | 战斗命中事件 | `hit_confirmed` |
| `game_` | 全局游戏状态 | `game_paused` |

### 通信规则（严格约束）

1. **跨层通信 = SignalBus** — 不同层之间的通信只走 SignalBus
2. **同层通信 = 直接调用** — 同一层内的系统可直接引用（如 Player → ComboManager）
3. **信号单向** — 发射方不知道谁在监听，接收方不知道谁在发射
4. **禁止信号链** — 接收方不得在回调中立即发射另一个 SignalBus 信号（防止无限循环）
5. **信号参数 ≤ 4 个** — 超过 4 个参数说明需要传递 Dictionary，但必须定义类型

### 同层 vs 跨层判断

| 场景 | 方式 | 原因 |
|------|------|------|
| Player → ComboManager | 直接引用 | 同一 Gameplay 层 |
| CombatSystem → HUD | SignalBus | 跨层（Gameplay → Presentation） |
| HitDetection → DamageNumber | SignalBus | 跨层（Core → Presentation） |
| EnemyAI → DungeonRoom | SignalBus | 跨层（Gameplay → Gameplay 但不同子系统） |
| PlayerController → InputBuffer | 直接引用 | 同一 Core 层 |

## Alternatives Considered

### Alternative 1: Direct Node References
- **Description**: 系统间通过 `get_node("/root/Main/HUD")` 或 `@export` 引用直接调用方法
- **Pros**: 调用栈清晰，IDE 可追踪；性能最优（直接方法调用）
- **Cons**: 强耦合——HUD 重命名会破坏 CombatSystem；场景切换时引用失效；新增监听者需修改发射方代码
- **Rejection Reason**: 违反"新增监听者不需修改发射方"的需求，且 Godot 场景树路径脆弱

### Alternative 2: Observer Pattern (手动实现)
- **Description**: 每个系统维护自己的监听者列表，手动注册/注销
- **Pros**: 不依赖 Autoload；监听者可限定范围
- **Cons**: 每个系统重复实现注册/注销逻辑；忘记注销导致内存泄漏；比 SignalBus 样板代码多
- **Rejection Reason**: Godot 信号系统已是 Observer 模式的引擎级实现，重复造轮子

### Alternative 3: Event Queue (异步消息队列)
- **Description**: 消息放入队列，下一帧统一分发
- **Cons**: 格斗游戏不能容忍 1 帧延迟——命中反馈必须在当帧显示；增加调度复杂度
- **Rejection Reason**: 战斗反馈 < 1帧延迟的需求排除了异步方案

## Consequences

### Positive
- **完全解耦** — 发射方和接收方互不知道对方存在
- **易扩展** — 新增 HUD 元素只需监听已有信号，零修改发射方
- **Godot 原生** — 信号是 Godot 核心机制，性能优异，调试工具支持
- **可测试** — 测试中只需连接信号到断言函数

### Negative
- **调试困难** — 信号调用栈不如直接调用清晰；需要手动追踪"谁连了这个信号"
- **隐式依赖** — 信号连接不在构造函数中体现，需查阅代码才能理解系统间关系
- **信号爆炸风险** — 随系统增多，SignalBus 可能积累过多信号

### Risks
- **风险**: 信号名冲突（两个系统定义同名信号）→ **缓解**: 严格前缀命名规范（player_ / combo_ / enemy_ 等）
- **风险**: 信号链导致无限循环 → **缓解**: 禁止在 SignalBus 回调中发射另一个 SignalBus 信号
- **风险**: 信号过多导致 SignalBus 难以维护 → **缓解**: MVP 范围限定为上述清单，新增信号需 code review

## GDD Requirements Addressed

| GDD System | Requirement | How This ADR Addresses It |
|------------|-------------|--------------------------|
| combat-system.md | 攻击命中时通知 HUD 和伤害数字 | `hit_confirmed` 信号 → HUD 监听更新连击，`damage_number_requested` → 伤害数字 |
| combo-system.md | 连击数和连招名实时同步到 HUD | `combo_updated(count, chain_name)` 信号 |
| hud.md | 每帧消费 HP/能量/连击数据但不直接引用战斗系统 | 监听 `player_health_changed`、`player_energy_changed`、`combo_updated` |
| enemy-ai.md | 敌人死亡通知房间系统 | `enemy_died(enemy)` 信号 → DungeonRoom 监听减少存活敌人数 |
| dungeon-flow.md | 房间清空/副本完成通知 HUD | `room_cleared(rating)`、`dungeon_cleared(id, stats)` 信号 |
| hit-detection-feedback.md | 命中时触发屏震、命中停顿、伤害数字 | `hit_confirmed` → ScreenShake 监听；`damage_number_requested` → DamageNumber 监听 |

## Performance Implications
- **CPU**: Godot 信号分发是 O(listeners) — 每个信号约 0.001ms/监听者，6 个监听者 < 0.01ms
- **Memory**: SignalBus 无状态，仅信号声明，内存 < 1KB
- **Load Time**: 无额外加载开销
- **Network**: 不适用

## Migration Plan
无 — 这是新项目的初始架构决策。

## Validation Criteria
1. SignalBus.gd 中所有信号都有类型注解（GDScript 2.0 typed signals）
2. 无信号名冲突——所有信号遵循前缀命名规范
3. 无信号链——任何 SignalBus 回调中不包含 `SignalBus.xxx.emit()` 调用
4. 跨层通信 100% 走 SignalBus，同层通信使用直接引用
5. 每个信号有 ≤ 4 个参数

## Related Decisions
- ADR-002: Autoload Singletons (SignalBus 是 4 个 Autoload 之一)
- ADR-001: GDScript as Primary Language (typed signals 是 GDScript 2.0 特性)
