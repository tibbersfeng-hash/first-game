# ADR-004: Resource-Based Data Configuration

## Status
Proposed

## Date
2026-06-15

## Engine Compatibility

| Field | Value |
|-------|-------|
| **Engine** | Godot 4.6.3 |
| **Domain** | Core |
| **Knowledge Risk** | HIGH — Godot 4.6 is post-LLM-cutoff |
| **References Consulted** | `docs/engine-reference/godot/VERSION.md`, `docs/engine-reference/godot/breaking-changes.md`, `docs/engine-reference/godot/deprecated-apis.md` |
| **Post-Cutoff APIs Used** | `duplicate_deep()` (Godot 4.5+) — 用于运行时深拷贝 Resource 实例 |
| **Verification Required** | 确认 .tres 文本格式在 Git 中可 diff；验证 Resource 在 _ready() 前完成加载 |

## ADR Dependencies

| Field | Value |
|-------|-------|
| **Depends On** | ADR-001 (GDScript as Primary Language — Resource 是 GDScript 原生特性) |
| **Enables** | ADR-002 (CombatData 从 .tres 加载帧数据); Enemy AI (EnemyData Resource); Dungeon System (DungeonData Resource) |
| **Blocks** | Enemy AI GDD implementation (needs EnemyData Resource); CombatData implementation |
| **Ordering Note** | 必须在任何数据驱动的系统实现之前决定，影响 CombatData、EnemyData、ComboData 等全部配置类 |

## Context

### Problem Statement
格斗萌主的数值配置种类繁多：攻击帧数据、敌人属性、连招规则、副本结构、掉落表等。这些数据需要在 Godot 编辑器中可视编辑、运行时高效访问、版本控制中可追踪变更。如何组织所有游戏配置数据？

### Constraints
- 策划/设计师需要在不改代码的情况下调整数值（帧数据、伤害、敌人属性等）
- 战斗系统每帧读取帧数据，访问延迟必须 < 0.01ms
- 配置变更需要在 Git 中可 diff，支持 code review
- 同类数据（如多个敌人）需要统一 schema 但各自独立实例

### Requirements
- 配置数据类型安全，有编译期检查
- Godot 编辑器中原生可视编辑，无需自定义工具
- 运行时只读，防止代码意外修改配置
- 版本控制友好（文本格式，可 diff）

## Decision

所有游戏配置数据使用 **Godot Resource** (.tres/.res) 格式存储。每个数据类型定义一个自定义 Resource 类，Godot 编辑器自动生成属性面板，策划直接在编辑器中编辑。

### 架构图

```
┌─────────────────────────────────────────────────┐
│                 Resource Type Hierarchy          │
│                                                  │
│  Resource (Godot built-in)                       │
│  ├── AttackData      # 单次攻击的帧数据          │
│  ├── ComboData       # 连招链定义                │
│  ├── EnemyData       # 敌人属性+行为配置          │
│  ├── DungeonData     # 副本结构+房间序列          │
│  ├── RoomData        # 房间波次+敌人配置          │
│  ├── LootTableData   # 掉落表                    │
│  └── EnergyConfig    # 能量系统参数               │
│                                                  │
│  Storage: src/infrastructure/data/               │
│    ├── enemies/  (EnemyData .tres files)         │
│    ├── combos/   (ComboData .tres files)         │
│    ├── dungeons/ (DungeonData .tres files)       │
│    └── rooms/    (RoomData .tres files)          │
└─────────────────────────────────────────────────┘
```

### 核心 Resource 定义

```gdscript
# ─── AttackData ───
class_name AttackData
extends Resource

@export var attack_id: String = ""
@export var display_name: String = ""
@export var startup_frames: int = 0
@export var active_frames: int = 0
@export var recovery_frames: int = 0
@export var damage: int = 0
@export var energy_cost: int = 0
@export var energy_gain_on_hit: int = 0
@export var cancel_priority: int = 0
@export var hitbox_offset: Vector2 = Vector2.ZERO
@export var hitbox_size: Vector2 = Vector2(32, 32)

# ─── EnemyData ───
class_name EnemyData
extends Resource

@export var enemy_id: String = ""
@export var display_name: String = ""
@export var max_hp: int = 100
@export var base_damage: int = 10
@export var move_speed: float = 100.0
@export var attack_range: float = 50.0
@export var chase_range: float = 200.0
@export var decision_interval: float = 1.0
@export var attack_data: Array[AttackData] = []
@export var loot_table: LootTableData = null
@export var category: String = "minion"  # minion | elite | boss

# ─── ComboData ───
class_name ComboData
extends Resource

@export var combo_id: String = ""
@export var display_name: String = ""
@export var chain: Array[String] = []  # attack_id sequence
@export var cancel_window_pct: float = 0.5  # recovery帧的前50%可取消
@export var bonus_damage_pct: float = 0.0

# ─── EnergyConfig ───
class_name EnergyConfig
extends Resource

@export var max_energy: int = 100
@export var starting_energy: int = 0
@export var special_threshold: int = 30
@export var dodge_cost: int = 8
@export var heavy_cost: int = 5
@export var hurt_gain: int = 3
```

### 数据加载流程

```
Game Start → CombatData._ready()
               │
               ├─→ preload("res://src/infrastructure/data/combos/basic_combo.tres")
               ├─→ preload("res://src/infrastructure/data/enemies/goblin.tres")
               └─→ preload("res://src/infrastructure/data/dungeons/forest_dungeon.tres")
               │
               └─→ 存入 Dictionary 缓存，运行时 O(1) 查询
```

### 使用规则

1. **所有 @export 变量有默认值** — 缺失 .tres 时不会崩溃
2. **运行时只读** — 代码不修改 Resource 属性，如需运行时状态用独立变量
3. **用 preload 不用 load** — preload 在编译期解析，零运行时 I/O
4. **.tres 文本格式入库** — 确保版本控制可 diff
5. **嵌套 Resource** — EnemyData 引用 AttackData 数组，Godot 编辑器自动支持嵌套编辑

## Alternatives Considered

### Alternative 1: JSON Configuration Files
- **Description**: 所有配置存为 .json 文件，运行时用 JSON.parse() 加载
- **Pros**: Git diff 最友好；跨工具通用；非程序员也能编辑
- **Cons**: 无类型检查——字段名拼错运行时才报错；需手写解析代码；Godot 编辑器不支持 JSON 编辑；嵌套结构解析繁琐
- **Rejection Reason**: 缺少类型安全是致命缺陷——战斗帧数据拼错一个字段名可能导致运行时崩溃

### Alternative 2: CSV/Spreadsheet
- **Description**: 数值用 Google Sheets 维护，导出 CSV 加载
- **Pros**: 策划最熟悉的工具；批量编辑方便
- **Cons**: 不支持嵌套结构（敌人→攻击数据→帧数据）；需导入/导出流程；CSV 无法表达复杂类型
- **Rejection Reason**: 格斗游戏的配置是深度嵌套结构（Enemy→Attacks→Frames），CSV 无法表达

### Alternative 3: Inline Code Constants
- **Description**: 所有数值定义为 GDScript 常量
- **Pros**: 类型安全；IDE 自动补全；零加载开销
- **Cons**: 修改数值需改代码+重启；策划无法独立编辑；无法热重载
- **Rejection Reason**: 违反"策划不改代码即可调整数值"的需求

## Consequences

### Positive
- **类型安全** — @export 变量有类型注解，拼错字段编辑器即报错
- **编辑器原生** — Godot Inspector 自动生成属性面板，策划直接编辑
- **嵌套支持** — Resource 可引用 Resource，天然支持 EnemyData→AttackData 层级
- **版本控制** — .tres 文本格式可 Git diff，变更可追踪

### Negative
- **.tres 可读性一般** — 文本格式不如 JSON 直观，嵌套时尤其冗长
- **合并冲突** — 两人同时编辑同一 .tres 可能产生 Git 合并冲突
- **非程序员门槛** — 需学习 Godot 编辑器的 Resource 编辑流程

### Risks
- **风险**: .tres 文本格式合并冲突难解决 → **缓解**: 每个敌人/连招/副本独立 .tres 文件，减少冲突面
- **风险**: 运行时代码意外修改 Resource → **缓解**: 代码 review 检查直接赋值 Resource 属性的写法
- **风险**: 嵌套 Resource 引用丢失 → **缓解**: 使用 preload 硬引用，避免 load 的懒加载风险

## GDD Requirements Addressed

| GDD System | Requirement | How This ADR Addresses It |
|------------|-------------|--------------------------|
| combat-system.md | 帧数据从配置加载，不硬编码 | AttackData Resource 定义 startup/active/recovery 帧数 |
| combat-system.md | 能量系统参数可调 | EnergyConfig Resource 定义 max_energy、costs、thresholds |
| combo-system.md | 连招链从配置定义 | ComboData Resource 定义 chain 序列和 cancel_window_pct |
| enemy-ai.md | 行为参数数据驱动，策划可调 | EnemyData Resource 定义 decision_interval、attack_range、chase_range |
| enemy-ai.md | 同类敌人不同参数 | 同一 EnemyData 类，不同 .tres 实例（goblin.tres vs goblin_elite.tres） |
| dungeon-flow.md | 副本结构可配置 | DungeonData + RoomData Resource 定义房间序列和波次 |

## Performance Implications
- **CPU**: Resource 属性访问 = 直接内存读取 — O(1)，< 0.001ms/次
- **Memory**: 所有 .tres preload 进内存，预估 MVP < 500KB
- **Load Time**: preload 在场景加载时完成，零运行时 I/O
- **Network**: 不适用

## Migration Plan
无 — 新项目初始决策。

## Validation Criteria
1. 所有自定义 Resource 类有 `class_name` 声明和 `extends Resource`
2. 所有 @export 变量有类型注解和默认值
3. 运行时代码不直接修改 Resource 属性（只读访问）
4. .tres 文件使用文本格式（project.godot 中设置）
5. 每个数据实体独立 .tres 文件，避免单文件过大

## Related Decisions
- ADR-001: GDScript as Primary Language (Resource 是 GDScript 原生特性)
- ADR-002: Autoload Singletons (CombatData 从 Resource 加载配置)
