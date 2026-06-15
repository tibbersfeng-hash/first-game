# Control Manifest

> **Engine**: Godot 4.6.3
> **Last Updated**: 2026-06-15
> **Manifest Version**: 2026-06-15
> **ADRs Covered**: ADR-001, ADR-002, ADR-003, ADR-004, ADR-005, ADR-006
> **Status**: Active — regenerate with `/create-control-manifest update` when ADRs change

This manifest is a programmer's quick-reference extracted from all Accepted ADRs,
technical preferences, and engine reference docs. For the reasoning behind each
rule, see the referenced ADR.

---

## Foundation Layer Rules

*Applies to: scene management, event architecture, engine initialisation, global services*

### Required Patterns

- **Use GDScript as primary language** — All game logic in GDScript; C# only for future performance-critical modules via GDExtension — source: ADR-001
- **Use Autoload singletons for cross-cutting services** — CombatData, GameManager, SignalBus, AudioManager are Autoload singletons — source: ADR-002
- **Register Autoloads in fixed load order** — CombatData → SignalBus → GameManager → AudioManager in project.godot — source: ADR-002
- **CombatData is read-only at runtime** — No system may modify CombatData properties during gameplay — source: ADR-002
- **GameManager is sole writer of game state** — Only GameManager itself may modify current_dungeon_id, current_room_index, revive_coins_remaining — source: ADR-002
- **GameManager must provide reset/start_dungeon()** — State must be explicitly reset between game sessions — source: ADR-002
- **SignalBus is stateless** — SignalBus only declares and forwards signals; it must never hold game state — source: ADR-002
- **AudioManager is command-only** — Accepts play requests, never calls back into business logic — source: ADR-002
- **Use SignalBus for all cross-layer communication** — Systems in different architectural layers must communicate via SignalBus signals — source: ADR-003
- **Signal parameters must be ≤ 4** — If more data is needed, pass a typed Dictionary — source: ADR-003
- **All signal names must follow prefix convention** — player_, combo_, enemy_, room_/dungeon_, hit_/attack_/damage_, game_ — source: ADR-003
- **All config data uses Godot Resource (.tres/.res)** — No JSON, no CSV, no inline constants for game data — source: ADR-004
- **Every custom Resource must have class_name + extends Resource** — Enables type checking and editor integration — source: ADR-004
- **All @export variables must have type annotations and default values** — Missing .tres must not crash the game — source: ADR-004
- **Use preload() not load() for config data** — preload resolves at compile time, zero runtime I/O — source: ADR-004
- **.tres files must use text format** — Required for Git diff and code review — source: ADR-004

### Forbidden Approaches

- **Never use C# as primary language** — GDScript's iteration speed and Godot integration are more important for this 2D fighter — source: ADR-001
- **Never mix GDScript + C#** — Two languages create cognitive overhead and interop bugs; use GDExtension for isolated performance needs instead — source: ADR-001
- **Never allow Autoloads to reference each other** — Only SignalBus may be listened to by other Autoloads; no direct cross-Autoload method calls — source: ADR-002
- **Never create a God Object Autoload** — Each Autoload has a single, strict responsibility — source: ADR-002
- **Never use Dependency Injection instead of Autoload** — DI is not idiomatic Godot; adds boilerplate with minimal benefit — source: ADR-002
- **Never use direct node references for cross-layer communication** — `get_node("/root/Main/HUD")` is fragile and couples systems — source: ADR-003
- **Never use event queues (async) for combat events** — Combat feedback must arrive in the same frame; 1-frame delay is unacceptable — source: ADR-003
- **Never emit a SignalBus signal from a SignalBus handler** — This creates infinite loop risk (signal chains) — source: ADR-003
- **Never use JSON for game configuration** — No type checking, requires manual parsing, no editor integration — source: ADR-004
- **Never use CSV for game configuration** — Cannot express nested data structures (Enemy→Attacks→Frames) — source: ADR-004
- **Never hardcode game values as code constants** — Values must be editable without code changes — source: ADR-004
- **Never mutate Resource properties at runtime** — Resources are read-only configuration; runtime state uses separate variables — source: ADR-004
- **Never use `duplicate()` for nested Resources** — Use `duplicate_deep()` (Godot 4.5+) — source: deprecated-apis.md

### Performance Guardrails

- **CombatData access**: max 0.01ms/frame (Dictionary.get O(1)) — source: ADR-002
- **4 Autoloads total memory**: < 2MB — source: ADR-002
- **CombatData preload time**: < 50ms — source: ADR-002
- **SignalBus dispatch**: < 0.01ms/signal (6 listeners) — source: ADR-003
- **Resource property access**: < 0.001ms/access — source: ADR-004
- **All config data in memory**: < 500KB (MVP scope) — source: ADR-004

---

## Core Layer Rules

*Applies to: core gameplay loop, main player systems, physics, collision, combat timing*

### Required Patterns

- **Use frame counts (integer) for all combat timing** — startup_frames, active_frames, recovery_frames are integers; no delta accumulation — source: ADR-006
- **All combat logic runs in _physics_process** — Never in _process; _physics_process runs at fixed 60 ticks/s — source: ADR-006
- **Set Engine.physics_ticks_per_second = 60** — Fixed in project.godot — source: ADR-006
- **Use frame_in_state counter per entity** — Increment each _physics_process; reset to 0 on state transition — source: ADR-006
- **Define attack phases by frame ranges** — startup: frame < startup_frames; active: frame < startup+active; recovery: rest — source: ADR-006
- **Cancel window = first 50% of recovery frames** — `frame >= recovery_start AND frame < recovery_start + int(recovery * 0.5)` — source: ADR-006
- **Room size is fixed 1280×720** — No camera scrolling, no zoom — source: ADR-005
- **Camera2D position locked at (640, 360)** — Center of room; smoothing disabled — source: ADR-005
- **Camera2D limits set to room bounds** — limit_left=0, limit_right=1280, limit_top=0, limit_bottom=720 — source: ADR-005
- **Use StaticBody2D walls for room boundaries** — Prevents player and enemies from leaving the room — source: ADR-005
- **Set physics_interpolation = false** — Combat logic doesn't need interpolation — source: ADR-006
- **Set default_gravity = 0** — 2D fighter uses predefined jump arcs, not physics gravity — source: ADR-006

### Forbidden Approaches

- **Never use delta time for combat timing** — Float accumulation causes drift and inconsistent cancel windows — source: ADR-006
- **Never put combat logic in _process()** — _process frame rate is variable; combat must be fixed-step — source: ADR-006
- **Never use Camera2D follow/scroll** — Fixed screen combat; DNF-style one-screen rooms — source: ADR-005
- **Never use dynamic zoom** — Destroys pixel-perfect rendering and distance judgment — source: ADR-005
- **Never use `_process()` for complex computation** — Combat logic goes in `_physics_process()` — source: technical-preferences.md
- **Never hardcode game values** — Must load from Resource configuration — source: technical-preferences.md
- **Never use `call_deferred` to bypass lifecycle issues** — Fix the architecture instead — source: technical-preferences.md

### Performance Guardrails

- **Player update**: max 1.0ms/frame — source: ADR-006
- **Combat system**: max 0.5ms/frame (hit detection + damage calc + combo check) — source: ADR-006
- **Enemies (×10)**: max 3.0ms/frame — source: ADR-002
- **Total game frame**: max 12ms (leaving 4ms for rendering) — source: architecture.md
- **Room transition**: max 100ms — source: architecture.md

---

## Feature Layer Rules

*Applies to: combat system, combo system, enemy AI, dungeon systems*

### Required Patterns

- **Same-layer systems may use direct references** — Player → ComboManager within Gameplay layer is OK — source: ADR-003
- **Cross-subsystem communication uses SignalBus** — EnemyAI → DungeonRoom even within Gameplay layer — source: ADR-003
- **Enemy behavior must be data-driven** — All enemy parameters come from EnemyData Resource, not hardcoded — source: ADR-004
- **Same entity type, different .tres instances** — Goblin vs Goblin Elite = same EnemyData class, different .tres files — source: ADR-004
- **Combo chains defined in ComboData Resource** — chain: Array[String] of attack_id sequence — source: ADR-004
- **Energy config from EnergyConfig Resource** — max_energy, thresholds, costs from .tres — source: ADR-004
- **Attack frame data from AttackData Resource** — startup/active/recovery frames as @export ints — source: ADR-004

### Forbidden Approaches

- **Never write to another system's state directly** — Use signals or method calls on the owner — source: ADR-002
- **Never implement manual Observer pattern** — Godot signals already provide this; don't reinvent — source: ADR-003
- **Never use hybrid frame+delta timing** — One timing system (frames) for all combat logic — source: ADR-006

### Performance Guardrails

- **Enemy AI decision interval**: configurable per enemy (0.3s-2.0s), not per-frame — source: enemy-ai GDD

---

## Presentation Layer Rules

*Applies to: HUD, visual effects, audio, animations*

### Required Patterns

- **HUD uses CanvasLayer** — Not affected by Camera2D position — source: ADR-005
- **HUD layout designed for 1280×720** — HP bar left-top, skills right-bottom, combo center-top — source: ADR-005
- **HUD updates are signal-driven** — Listen to SignalBus events; no polling — source: ADR-003
- **HUD animations ≤ 0.3 seconds** — Must not interfere with combat reading — source: hud GDD
- **All HUD elements semi-transparent** — Background alpha 0.6-0.8; combat area always visible — source: hud GDD

### Forbidden Approaches

- **Never reference combat systems directly from HUD** — HUD only consumes SignalBus events — source: ADR-003
- **Never manipulate UI from character scripts** — Use signals to decouple — source: technical-preferences.md
- **Never use `$NodePath` in _process()** — Cache references with `@onready var` — source: deprecated-apis.md

### Performance Guardrails

- **HUD update**: max 0.5ms/frame — source: ADR-003

---

## Global Rules (All Layers)

### Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes | PascalCase | `PlayerController`, `ComboManager`, `DungeonRoom` |
| Variables | snake_case | `combo_count`, `base_damage`, `move_speed` |
| Signals/Events | snake_case past-tense | `hit_landed`, `combo_finished`, `room_cleared` |
| Files | snake_case | `player_controller.gd`, `combo_manager.gd` |
| Scenes/Prefabs | PascalCase | `Player.tscn`, `DungeonRoom.tscn`, `BossGoblin.tscn` |
| Constants | UPPER_SNAKE_CASE | `MAX_COMBO_COUNT`, `BASE_DAMAGE`, `HIT_STOP_FRAMES` |

### Performance Budgets

| Target | Value |
|--------|-------|
| Framerate | 60 FPS (combat scenes must be stable) |
| Frame budget | 16.67ms — combat logic ≤ 5ms, rendering ≤ 10ms |
| Draw calls | < 100 per frame (2D sprite scene) |
| Memory ceiling | < 512MB |

### Approved Libraries / Addons

- **GUT** (Godot Unit Testing) — unit test framework
- **Godot 4.6 built-in** — Tween, AnimationPlayer, StateMachine, etc.

### Forbidden APIs (Godot 4.6.3)

These APIs are deprecated and must not be used:

| Forbidden API | Use Instead | Since |
|---------------|-------------|-------|
| `yield()` | `await signal` | 4.0 |
| `connect("signal", obj, "method")` | `signal.connect(callable)` | 4.0 |
| `instance()` / `PackedScene.instance()` | `instantiate()` / `PackedScene.instantiate()` | 4.0 |
| `OS.get_ticks_msec()` | `Time.get_ticks_msec()` | 4.0 |
| `duplicate()` for nested Resources | `duplicate_deep()` | 4.5 |
| `TileMap` | `TileMapLayer` | 4.3 |
| `YSort` | `Node2D.y_sort_enabled` | 4.0 |
| `VisibilityNotifier2D/3D` | `VisibleOnScreenNotifier2D/3D` | 4.0 |
| String-based `connect()` | Typed signal connections | 4.0 |
| `$NodePath` in `_process()` | `@onready var` cached reference | Pattern |
| Untyped `Array` / `Dictionary` | `Array[Type]`, typed variables | Pattern |
| `Texture2D` in shader parameters | `Texture` base type | 4.4 |

Source: `docs/engine-reference/godot/deprecated-apis.md`

### Cross-Cutting Constraints

- **No combat logic in _process()** — All timing-critical code runs in _physics_process at 60Hz
- **No hardcoded game values** — All numerical data loaded from Resource (.tres) configuration files
- **No direct UI manipulation from game logic** — Use SignalBus events to decouple
- **No call_deferred to work around lifecycle bugs** — Fix the architecture instead
- **Frame-based timing everywhere in combat** — Integer frame counters; never delta accumulation
- **Signal chain prohibition** — A SignalBus handler must never emit another SignalBus signal
- **Resource immutability** — Runtime code must never modify Resource properties
