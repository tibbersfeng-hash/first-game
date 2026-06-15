# ADR-002: Autoload Singletons for Cross-Cutting Services

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
| **References Consulted** | `docs/engine-reference/godot/VERSION.md`, `docs/engine-reference/godot/breaking-changes.md` |
| **Post-Cutoff APIs Used** | None — Autoload is a stable Godot feature since 3.x |
| **Verification Required** | Confirm Autoload load order in project.godot; verify no circular dependencies between Autoloads |

## ADR Dependencies

| Field | Value |
|-------|-------|
| **Depends On** | ADR-001 (GDScript as Primary Language — Accepted) |
| **Enables** | ADR-003 (Signal Bus requires SignalBus Autoload) |
| **Blocks** | All Gameplay and Presentation systems that need global services |
| **Ordering Note** | Must be decided before Signal Bus ADR, as SignalBus is itself an Autoload |

## Context

### Problem Statement
格斗萌主的多个系统需要全局可访问的服务：CombatData 提供战斗配置、GameManager 追踪游戏状态、AudioManager 播放音效、SignalBus 转发跨系统事件。这些服务需要在任何场景、任何节点中可用，且生命周期跨越整个游戏运行期。如何提供这些跨层全局服务？

### Constraints
- Godot 场景切换会释放当前场景树中的所有节点，但全局服务必须跨场景持久
- 战斗系统每帧读取配置数据（帧数据、取消窗口），访问路径必须零开销
- 多个系统（Player, Enemy, HUD, DungeonFlow）需要消费同一服务的数据
- 独立开发者维护，架构必须简单直觉

### Requirements
- 全局服务必须跨场景持久存活
- 战斗配置数据读取必须 O(1)，无每帧路径查找
- 服务之间必须低耦合，避免形成 God Object
- 必须可单元测试（可注入/可替换）

## Decision

使用 Godot **Autoload 单例**提供 4 个跨层全局服务，每个 Autoload 职责严格限定为单一领域：

| Autoload | 职责 | 数据流向 | 写入者 |
|----------|------|---------|--------|
| **CombatData** | 战斗配置数据（帧数据、取消规则、伤害公式） | 只读 | 编辑器 .tres 文件 |
| **GameManager** | 游戏运行时状态（当前副本、房间进度、复活币） | 读写 | GameManager 自身 |
| **SignalBus** | 跨系统事件信号转发 | 事件流 | 任意发射者 |
| **AudioManager** | 音频播放（BGM、SFX、战斗音效） | 命令式 | 任意调用者 |

### 架构图

```
┌─────────────────────────────────────────────────┐
│                  Autoload Layer                  │
│                                                  │
│  ┌────────────┐ ┌──────────┐ ┌──────────────┐  │
│  │ CombatData │ │GameManager│ │  SignalBus   │  │
│  │ (read-only)│ │(read-write)│ │ (event bus) │  │
│  └─────┬──────┘ └─────┬────┘ └──────┬───────┘  │
│        │               │             │           │
│  ┌─────┴──────┐        │             │           │
│  │AudioManager│        │             │           │
│  │ (commands) │        │             │           │
│  └────────────┘        │             │           │
└────────┬───────────────┼─────────────┼───────────┘
         │               │             │
    ┌────▼────┐    ┌─────▼─────┐ ┌────▼────┐
    │CombatSys│    │DungeonFlow│ │All Sys. │
    │EnemyAI  │    │HUD        │ │(signals)│
    └─────────┘    └───────────┘ └─────────┘
```

### 关键接口

```gdscript
# ─── CombatData (Autoload) ───
# 只读配置，运行时不可修改
extends Node

var frame_data: Dictionary = {}  # 预加载的帧数据表
var cancel_rules: Dictionary = {}  # 取消规则表
var energy_config: Dictionary = {}  # 能量系统配置

func _ready() -> void:
    # 从 .tres Resource 加载所有配置
    _load_combat_data()

func get_attack_frames(attack_id: String) -> Dictionary:
    return frame_data.get(attack_id, {})

# ─── GameManager (Autoload) ───
# 游戏运行时状态管理
extends Node

signal game_state_changed(new_state: String)

var current_dungeon_id: String = ""
var current_room_index: int = 0
var revive_coins_remaining: int = 0
var game_state: String = "menu"  # menu | playing | paused | result

func start_dungeon(dungeon_id: String) -> void: ...
func advance_room() -> void: ...
func use_revive_coin() -> bool: ...

# ─── SignalBus (Autoload) ───
# 详见 ADR-003
# 仅声明信号，不持有状态

# ─── AudioManager (Autoload) ───
# 命令式音频播放
extends Node

func play_sfx(stream: AudioStream, position: Vector2 = Vector2.ZERO) -> void: ...
func play_bgm(stream: AudioStream, fade_time: float = 1.0) -> void: ...
func stop_bgm(fade_time: float = 1.0) -> void: ...
func play_combat_hit_sfx(damage_type: int) -> void: ...
```

### Autoload 使用规则（严格约束）

1. **CombatData**: 只读 — 任何系统不可在运行时修改其数据
2. **GameManager**: 唯一写入者 — 只有 GameManager 自身可修改其状态，其他系统通过信号通知 GameManager 做变更
3. **SignalBus**: 无状态 — 只声明和转发信号，不持有任何游戏状态
4. **AudioManager**: 命令式 — 接受播放请求，不回调业务逻辑
5. **禁止 Autoload 间互相引用** — CombatData 不引用 GameManager，AudioManager 不引用 SignalBus。唯一例外：SignalBus 可被所有 Autoload 监听

## Alternatives Considered

### Alternative 1: Dependency Injection via Node References
- **Description**: 不用 Autoload，每个需要服务的节点通过 `@export` 或 `_enter_tree()` 时注入依赖
- **Pros**: 完全解耦，可单元测试，依赖显式
- **Cons**: 场景切换时需重新注入；Godot 编辑器不支持 DI 容器；战斗系统每帧获取引用增加复杂度；独立开发者维护成本高
- **Rejection Reason**: Godot 生态中 DI 不是惯用模式，增加大量样板代码且收益有限

### Alternative 2: Resource Singleton Pattern
- **Description**: 全局服务用 `Resource` + 静态引用实现，不用 Autoload
- **Pros**: 不依赖场景树，纯数据对象更容易测试
- **Cons**: Resource 不参与 `_process` 生命周期，无法自动管理帧更新；需要手动初始化和清理；无法利用 Godot 的场景树信号系统
- **Rejection Reason**: 无法满足 GameManager 需要帧更新和场景生命周期的需求

### Alternative 3: God Object (单一全局管理器)
- **Description**: 一个 Autoload `Game` 管理所有全局状态和逻辑
- **Pros**: 最简单，一个入口点
- **Cons**: 违反单一职责；所有系统耦合到同一对象；修改任何子功能都可能影响其他功能；无法独立测试任何子系统
- **Rejection Reason**: God Object 是已知反模式，维护性极差

## Consequences

### Positive
- **Godot 惯用模式** — Autoload 是 Godot 官方推荐的全局服务方案，社区资源和文档丰富
- **零开销访问** — Autoload 是全局名称，Godot 直接解析，无路径查找开销
- **生命周期简单** — Autoload 在游戏启动时创建，退出时销毁，跨场景持久
- **可测试性保留** — CombatData 只读 + SignalBus 无状态 + GameManager 状态可重置，支持测试隔离

### Negative
- **隐式依赖** — 节点代码中 `CombatData.xxx` 不在构造函数参数中体现，阅读代码时需知哪些 Autoload 存在
- **Autoload 加载顺序** — Godot 按 project.godot 中的注册顺序加载，顺序错误会导致空引用
- **全局状态风险** — GameManager 持有可变状态，需要显式重置机制防止跨场景状态泄漏

### Risks
- **风险**: Autoload 间循环依赖 → **缓解**: 禁止 Autoload 间互相引用（仅 SignalBus 可被监听）
- **风险**: GameManager 状态泄漏到下一局 → **缓解**: `start_dungeon()` 时显式重置所有状态
- **风险**: Autoload 加载顺序变更导致崩溃 → **缓解**: project.godot 中固定顺序：CombatData → SignalBus → GameManager → AudioManager

## GDD Requirements Addressed

| GDD System | Requirement | How This ADR Addresses It |
|------------|-------------|--------------------------|
| combat-system.md | 帧数据 O(1) 访问，每帧读取 startup/active/recovery 帧数 | CombatData Autoload 预加载帧数据表，提供 `get_attack_frames()` 直接查询 |
| combat-system.md | 能量系统配置（max=100, 必杀≥30, 闪避≥8） | CombatData 提供 energy_config 字典，不硬编码 |
| enemy-ai.md | 决策间隔数据驱动，从 EnemyData Resource 读取 | CombatData 提供通用战斗参数，EnemyData 由 Resource 系统加载（ADR-004） |
| dungeon-flow.md | 追踪当前房间索引、副本 ID、复活币余量 | GameManager 持有 current_dungeon_id、current_room_index、revive_coins_remaining |
| hud.md | 实时消费 HP/能量/连击数据 | HUD 通过 SignalBus 监听状态变更（ADR-003），不直接引用 GameManager |

## Performance Implications
- **CPU**: CombatData 读取是 Dictionary.get() — O(1) 均摊，每帧 < 0.01ms
- **Memory**: 4 个 Autoload 常驻内存，预估 < 2MB（配置数据 + 音频流引用）
- **Load Time**: Autoload 在游戏启动时加载，CombatData 预加载 .tres 约 < 50ms
- **Network**: 不适用

## Migration Plan
无 — 这是新项目的初始架构决策，无需迁移。

## Validation Criteria
1. 所有 Autoload 在 `project.godot` 中正确注册，加载顺序为 CombatData → SignalBus → GameManager → AudioManager
2. 没有任何 Autoload 在 `_ready()` 中引用其他 Autoload
3. CombatData 的所有方法均为只读（无 setter）
4. GameManager 提供 `reset()` / `start_dungeon()` 方法用于状态重置
5. 单元测试中可替换 Autoload（通过场景树临时添加同名节点）

## Related Decisions
- ADR-001: GDScript as Primary Language
- ADR-003: Signal Bus over Direct References (SignalBus 是本 ADR 中的 4 个 Autoload 之一)
- ADR-004: Resource-Based Data Configuration (CombatData 从 .tres 加载配置)
