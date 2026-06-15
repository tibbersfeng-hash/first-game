# Technical Preferences

<!-- Populated by /setup-engine. Updated as the user makes decisions throughout development. -->
<!-- All agents reference this file for project-specific standards and conventions. -->

## Engine & Language

- **Engine**: Godot 4.6.3
- **Language**: GDScript (主语言) + C# (可选，用于性能关键模块)
- **Rendering**: GLES3 (2D项目使用Forward+以获取完整着色器支持)
- **Physics**: Godot Physics (2D内置) / Jolt (4.6默认)

## Input & Platform

- **Target Platforms**: PC (Steam/Epic)
- **Input Methods**: Keyboard/Mouse + Gamepad
- **Primary Input**: Keyboard (连招搓招)，Gamepad 作为完整替代
- **Gamepad Support**: Full — 所有操作均可通过手柄完成
- **Touch Support**: None
- **Platform Notes**: PC首版，后期考虑移动端移植需重新设计输入

## Naming Conventions

- **Classes**: PascalCase — `PlayerController`, `ComboManager`, `DungeonRoom`
- **Variables**: snake_case — `combo_count`, `base_damage`, `move_speed`
- **Signals/Events**: snake_case 过去式 — `hit_landed`, `combo_finished`, `room_cleared`
- **Files**: snake_case — `player_controller.gd`, `combo_manager.gd`
- **Scenes/Prefabs**: PascalCase — `Player.tscn`, `DungeonRoom.tscn`, `BossGoblin.tscn`
- **Constants**: UPPER_SNAKE_CASE — `MAX_COMBO_COUNT`, `BASE_DAMAGE`, `HIT_STOP_FRAMES`

## Performance Budgets

- **Target Framerate**: 60 FPS (战斗场景必须稳定)
- **Frame Budget**: 16.67ms — 战斗逻辑不超过 5ms，渲染不超过 10ms
- **Draw Calls**: < 100 per frame (2D精灵场景)
- **Memory Ceiling**: < 512MB

## Testing

- **Framework**: GUT (Godot Unit Testing) + 手动场景测试
- **Minimum Coverage**: 80% 核心战斗逻辑（连招判定、伤害计算、状态机）
- **Required Tests**: 连招判定公式、伤害计算、状态机转换、装备词缀组合

## Forbidden Patterns

- 不在 `_process()` 中做复杂计算 — 战斗逻辑用 `_physics_process()`
- 不硬编码游戏数值 — 必须从 Resource/JSON 配置加载
- 不在角色脚本中直接操作 UI — 使用信号解耦
- 不使用 `call_deferred` 绕过生命周期问题 — 正确重构架构

## Allowed Libraries / Addons

- GUT (Godot Unit Testing)
- Godot 4.6 内置功能（Tween, AnimationPlayer, StateMachine 等）

## Architecture Decisions Log

<!-- Quick reference linking to full ADRs in docs/architecture/ -->
- [ADR-001: GDScript as Primary Language](../docs/architecture/adr-001-gdscript-primary.md) — Accepted
- [ADR-002: Autoload Singletons for Cross-Cutting Services](../docs/architecture/adr-002-autoload-singletons.md) — Proposed
- [ADR-003: Signal Bus over Direct References](../docs/architecture/adr-003-signal-bus.md) — Proposed
- [ADR-004: Resource-Based Data Configuration](../docs/architecture/adr-004-resource-data-configuration.md) — Proposed
- [ADR-005: Fixed Room Size (1280×720)](../docs/architecture/adr-005-fixed-room-size.md) — Proposed
- [ADR-006: Frame-Based Combat Timing](../docs/architecture/adr-006-frame-based-combat-timing.md) — Proposed

## Engine Specialists

- **Primary**: godot-specialist
- **Language/Code Specialist**: godot-gdscript-specialist
- **Shader Specialist**: godot-shader-specialist
- **UI Specialist**: godot-specialist (Godot UI 使用内置节点)
- **Additional Specialists**: godot-gdextension-specialist (如需C++性能模块)
- **Routing Notes**: 2D项目为主，shader需求集中在对打击反馈特效和技能视觉效果

### File Extension Routing

| File Extension / Type | Specialist to Spawn |
|-----------------------|---------------------|
| Game code (.gd) | godot-gdscript-specialist |
| Shader / material files (.gdshader) | godot-shader-specialist |
| UI / screen files (.tscn + Control节点) | godot-specialist |
| Scene / prefab / level files (.tscn) | godot-specialist |
| Native extension / plugin files (.cpp/.gdextension) | godot-gdextension-specialist |
| General architecture review | godot-specialist |
