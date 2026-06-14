# Architecture Document — 格斗萌主

> **Version**: 1.0
> **Last Updated**: 2026-06-15
> **Status**: Draft
> **Engine**: Godot 4.6.3
> **Language**: GDScript

## Architecture Overview

格斗萌主采用**分层架构**，自底向上分为5层：Framework → Core → Gameplay → Presentation → Infrastructure。每层只依赖其下层，禁止反向依赖。Godot 节点树和信号系统实现层间通信，Autoload 单例提供跨层服务。

```
┌─────────────────────────────────────────────┐
│           Infrastructure Layer              │  Autoloads: CombatData, AudioManager
├─────────────────────────────────────────────┤
│           Presentation Layer                │  HUD, DungeonFlow, DungeonRoom
├─────────────────────────────────────────────┤
│           Gameplay Layer                    │  CombatSystem, ComboSystem, EnemyAI
├─────────────────────────────────────────────┤
│           Core Layer                        │  PlayerController, HitDetection, InputBuffer
├─────────────────────────────────────────────┤
│           Framework Layer                   │  Godot Engine, Physics, SceneTree
└─────────────────────────────────────────────┘
```

## Module Dependency Graph

```
                    ┌──────────┐
                    │ CombatData│ (Autoload)
                    └─────┬────┘
                          │ (config values)
          ┌───────────────┼───────────────┐
          ▼               ▼               ▼
    ┌──────────┐   ┌──────────┐   ┌──────────┐
    │  Input   │   │  Player  │   │   Hit    │
    │  Buffer  │──▶│Controller│◀──▶│Detection │
    └──────────┘   └────┬─────┘   └────┬─────┘
                        │              │
              ┌─────────┼──────────────┤
              ▼         ▼              ▼
        ┌──────────┐ ┌──────────┐ ┌──────────┐
        │  Combat  │ │  Combo   │ │  Enemy   │
        │  System  │◀▶ System  │ │    AI    │
        └────┬─────┘ └──────────┘ └────┬─────┘
             │                          │
     ┌───────┼──────────┐              │
     ▼       ▼          ▼              ▼
┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐
│  HUD   │ │Dungeon │ │Dungeon │ │  Loot  │
│ System │ │  Room  │ │  Flow  │ │ System │
└────────┘ └────────┘ └────────┘ └────────┘
```

## Godot Scene Tree

```
Main (Node2D)
├── World (Node2D)                    # 游戏世界容器
│   ├── Camera2D (with ScreenShake)    # 摄像机+震动
│   ├── Floor/Platforms (StaticBody2D) # 地形
│   ├── Player (CharacterBody2D)       # 玩家角色
│   │   ├── BodySprite (AnimatedSprite2D)
│   │   ├── CollisionShape2D
│   │   ├── HitboxPivot/HitboxArea    # 攻击判定
│   │   ├── HurtboxArea               # 受击判定
│   │   ├── ComboManager (Node)
│   │   ├── HitStopManager (Node)
│   │   └── StateMachine (Node)
│   ├── Enemies (Node2D)              # 敌人容器
│   │   └── [Enemy instances]
│   └── Effects (Node2D)              # 特效容器
│       └── [DamageNumber, Particles]
├── HUD (CanvasLayer)                 # HUD层
│   ├── HPBar
│   ├── EnergyBar
│   ├── ComboCounter
│   ├── SkillBar
│   ├── BossHPBar
│   └── WaveIndicator
└── DungeonManager (Node)             # 副本流程管理
    └── RoomManager (Node)            # 房间管理

Autoloads:
├── CombatData (Node)                 # 战斗配置数据
├── GameManager (Node)                # 全局游戏状态
└── AudioManager (Node)               # 音频管理
```

## Core Systems Design

### 1. State Machine Pattern

所有有状态实体（Player, Enemy）使用统一的状态机模式：

```gdscript
class_name StateMachine
extends Node

@export var initial_state: State
var current_state: State
var states: Dictionary = {}

func _ready() -> void:
    for child in get_children():
        if child is State:
            states[child.name.to_lower()] = child
            child.state_machine = self
    if initial_state:
        current_state = initial_state
        current_state.enter()

func _physics_process(delta: float) -> void:
    if current_state:
        current_state.physics_update(delta)

func transition_to(state_name: String) -> void:
    var new_state = states.get(state_name.to_lower())
    if not new_state or new_state == current_state:
        return
    current_state.exit()
    current_state = new_state
    current_state.enter()
```

### 2. Signal Bus Pattern

跨系统通信通过信号总线，避免直接引用：

```gdscript
# Autoload: SignalBus
signal player_health_changed(current: int, maximum: int)
signal player_energy_changed(current: float, maximum: float)
signal combo_updated(count: int, chain_name: String)
signal combo_dropped
signal enemy_died(enemy: CharacterBody2D)
signal room_cleared(rating: String)
signal room_entered(room_id: String)
signal dungeon_cleared(dungeon_id: String, stats: Dictionary)
signal hit_confirmed(target: Node2D, damage: int, type: int, is_crit: bool)
signal damage_number_requested(pos: Vector2, damage: int, type: int, is_crit: bool)
```

### 3. Data-Driven Configuration

所有游戏数值从 Resource 加载，不硬编码：

```gdscript
class_name EnemyData
extends Resource

@export var enemy_id: String
@export var display_name: String
@export var max_hp: int
@export var base_damage: int
@export var move_speed: float
@export var attack_range: float
@export var chase_range: float
@export var decision_interval: float
@export var attack_data: Array[AttackData]
@export var loot_table: LootTable
```

### 4. Component Pattern

角色功能通过组合而非继承：

```
Player
├── Component: MovementComponent     # 移动逻辑
├── Component: CombatComponent       # 战斗逻辑
├── Component: HealthComponent       # 生命值管理
├── Component: EnergyComponent       # 能量管理
├── Component: InputBufferComponent  # 输入缓冲
└── Component: AnimationComponent   # 动画驱动
```

## Key Architectural Decisions

### ADR-001: GDScript as Primary Language
- **Decision**: 使用 GDScript 作为主语言，C# 仅用于性能关键模块
- **Rationale**: GDScript 与 Godot 深度集成，迭代速度快，社区资源丰富。2D 横版格斗对性能要求可通过优化解决
- **Consequences**: 性能热点需额外关注；后期可用 GDExtension 迁移热点

### ADR-002: Autoload Singletons for Cross-Cutting Services
- **Decision**: CombatData、GameManager、AudioManager 使用 Autoload
- **Rationale**: 战斗配置、游戏状态、音频需要全局访问，Autoload 是 Godot 的标准方案
- **Consequences**: 限制为只读服务+事件通知，禁止 Autoload 之间互相调用形成耦合

### ADR-003: Signal Bus over Direct References
- **Decision**: 跨层通信使用 SignalBus Autoload，而非直接节点引用
- **Rationale**: 解耦系统，支持未来添加监听者无需修改发射方
- **Consequences**: 信号过多时需分类命名避免混乱；调试时调用栈不如直接引用清晰

### ADR-004: Resource-Based Data Configuration
- **Decision**: 所有数值配置使用 Godot Resource (.tres/.res)，不用 JSON
- **Rationale**: Resource 有类型检查、编辑器支持、热重载；JSON 需要手动解析
- **Consequences**: 非程序员需学习 Godot 编辑器；版本控制中 .tres 是文本可 diff

### ADR-005: Fixed Room Size (1280×720, No Scrolling)
- **Decision**: 每个房间固定一屏，无摄像机跟随滚动
- **Rationale**: 简化实现、确保所有内容可见、匹配原型验证的战斗区域
- **Consequences**: 大型战斗场景需拆分；未来 Boss 战可能需要突破此限制

### ADR-006: Frame-Based Combat Timing
- **Decision**: 战斗系统使用帧数而非秒数定义所有时序
- **Rationale**: 格斗游戏手感依赖帧精确控制，60fps 下 1帧=16.67ms 是最小单位
- **Consequences**: 需要在 _physics_process 中处理，delta 不可靠时需帧计数器

## Directory Structure

```
src/
├── core/                           # Core Layer
│   ├── state_machine/              # 通用状态机
│   │   ├── state_machine.gd
│   │   └── state.gd
│   ├── input/                      # 输入系统
│   │   ├── input_buffer.gd         # 输入缓冲
│   │   └── input_context.gd        # 上下文管理
│   ├── hit_detection/              # 命中检测
│   │   ├── hitbox_component.gd     # 攻击框组件
│   │   ├── hurtbox_component.gd    # 受击框组件
│   │   ├── hit_stop_manager.gd     # 命中停顿
│   │   └── damage_number.gd        # 伤害数字
│   └── components/                 # 通用组件
│       ├── health_component.gd     # HP管理
│       ├── energy_component.gd     # 能量管理
│       └── movement_component.gd   # 移动逻辑
├── gameplay/                       # Gameplay Layer
│   ├── player/                     # 玩家相关
│   │   ├── player_controller.gd    # 主控制器
│   │   ├── player_combat.gd        # 战斗逻辑
│   │   └── states/                 # 玩家状态
│   │       ├── player_idle_state.gd
│   │       ├── player_run_state.gd
│   │       ├── player_jump_state.gd
│   │       ├── player_attack_state.gd
│   │       ├── player_dodge_state.gd
│   │       └── player_hitstun_state.gd
│   ├── combat/                     # 战斗系统
│   │   ├── combat_system.gd        # 战斗管理
│   │   └── combo_manager.gd        # 连招管理
│   ├── enemies/                    # 敌人相关
│   │   ├── enemy_base.gd           # 敌人基类
│   │   ├── minion_ai.gd            # 小怪AI
│   │   ├── boss_ai.gd              # Boss AI
│   │   └── states/                 # 敌人状态
│   └── dungeon/                    # 副本系统
│       ├── dungeon_manager.gd      # 副本流程
│       ├── room_manager.gd         # 房间管理
│       └── wave_manager.gd         # 波次管理
├── presentation/                   # Presentation Layer
│   ├── hud/                        # HUD
│   │   ├── hud_manager.gd          # HUD主控
│   │   ├── hp_bar.gd
│   │   ├── energy_bar.gd
│   │   ├── combo_counter.gd
│   │   ├── skill_bar.gd
│   │   └── boss_hp_bar.gd
│   └── effects/                    # 视觉特效
│       ├── screen_shake.gd
│       └── hit_flash.gd
├── infrastructure/                 # Infrastructure Layer
│   ├── autoloads/                  # 全局单例
│   │   ├── combat_data.gd          # 战斗配置
│   │   ├── game_manager.gd         # 游戏状态
│   │   ├── signal_bus.gd           # 信号总线
│   │   └── audio_manager.gd        # 音频管理
│   └── data/                       # 数据配置
│       ├── enemies/                # 敌人数据 .tres
│       ├── combos/                 # 连招数据 .tres
│       ├── dungeons/               # 副本数据 .tres
│       └── rooms/                  # 房间数据 .tres
└── utils/                          # 工具类
    ├── math_utils.gd
    └── debug_utils.gd
```

## Performance Budgets

| System | Budget | Target |
|--------|--------|--------|
| Player update | < 1ms | 60fps _physics_process |
| Enemy update (×10) | < 3ms | 10 enemies active |
| Combat system | < 0.5ms | Hit detection + feedback |
| HUD update | < 0.5ms | All bars + combo + skills |
| Room transition | < 100ms | Instant feel |
| Total game frame | < 12ms | Leave 4ms for rendering |

## Error Handling Strategy

| Error Type | Strategy | Example |
|-----------|----------|---------|
| Missing data | Log warning + use default | EnemyData not found → use default |
| Invalid state transition | Log error + stay in current | Player hitstun→jump blocked |
| Null reference | Guard + early return | Enemy target lost → return to patrol |
| Scene load failure | Fallback scene | Room scene missing → error room |
| Performance spike | Frame skip | Drop to 30fps rather than crash |

## Next Steps

- [ ] Create ADR files for each decision
- [ ] Implement core state machine
- [ ] Implement SignalBus Autoload
- [ ] Set up directory structure in src/
