# Technical Preferences

<!-- Updated for UE5 migration (2026-06-17). All agents reference this file. -->

## Engine & Language

- **Engine**: Unreal Engine 5.6
- **Language**: C++ (核心系统) + Blueprint (内容/关卡/动画)
- **Rendering**: 2.5D — 固定侧视相机，默认渲染管线
- **Physics**: UE5 Chaos Physics（2D约束：锁定Y轴平面运动）

## Input & Platform

- **Target Platforms**: PC (Steam/Epic), Linux
- **Input Methods**: Keyboard/Mouse + Gamepad
- **Primary Input**: Keyboard（连招搓招），Gamepad 作为完整替代
- **Input System**: Enhanced Input System
- **Gamepad Support**: Full — 所有操作均可通过手柄完成
- **Touch Support**: None

## Naming Conventions (UE5 C++)

- **Classes**: PascalCase + prefix — `APlayerCharacter`, `UCombatAbility`, `ADungeonRoom`
- **Variables**: camelCase — `currentHealth`, `baseDamage`, `moveSpeed`
- **UE Macros**: UPROPERTY, UFUNCTION, UCLASS, GENERATED_BODY
- **Functions**: PascalCase — `PerformLightAttack()`, `TakeDamage()`
- **Files**: PascalCase — `PlayerCharacter.h`, `HitBoxComponent.cpp`
- **Constants**: camelCase with k prefix — `kMaxComboCount`, `kBaseDamage`
- **Delegates**: F prefix + description — `FOnHitLanded`, `FOnComboFinished`
- **Enums**: E prefix + PascalCase — `EGameState`, `ERooroomType`

## Project Structure

```
src_ue5/
├── FirstGame.uproject
├── Config/
│   ├── DefaultGame.ini
│   ├── DefaultEngine.ini
│   └── DefaultInput.ini
├── Content/                    # Assets (Blueprints, meshes, textures)
├── Source/FirstGame/
│   ├── FirstGame.Build.cs
│   ├── Public/                 # Header files
│   │   ├── Subsystems/         # SignalBus, CombatData, GameManager
│   │   ├── Characters/         # PlayerCharacter, BaseEnemy
│   │   ├── Combat/             # HitBox, HurtBox, CombatAbility
│   │   ├── Input/              # Enhanced Input config
│   │   ├── AI/                 # EnemyAIController, Behavior Trees
│   │   ├── Dungeon/            # DungeonRoom, DungeonFlow
│   │   ├── UI/                 # HUDWidget (UMG)
│   │   ├── DataAssets/         # CharacterDataAsset, EnemyDataAsset
│   │   └── Tests/              # Automation tests
│   ── Private/                # Implementation files (mirror Public/)
```

## Performance Budgets

- **Target Framerate**: 60 FPS (战斗场景必须稳定)
- **Frame Budget**: 16.67ms — 战斗逻辑不超过 5ms
- **Draw Calls**: < 200 per frame (2.5D场景)
- **Memory Ceiling**: < 1GB

## Testing

- **Framework**: UE5 Automation Framework (C++ unit tests) + Gauntlet (integration)
- **Minimum Coverage**: 80% 核心战斗逻辑（连招判定、伤害计算、状态机）
- **Required Tests**: 连招判定公式、伤害计算、状态机转换、HitBox/HurtBox碰撞

## Forbidden Patterns

- 不在 Tick 中做复杂计算 — 战斗逻辑用 Timer 或 AnimNotify
- 不硬编码游戏数值 — 必须从 DataAsset 配置加载
- 不在角色类中直接操作 UI — 使用 SignalBus 解耦
- 不跳过 GAS 直接用自定义技能系统 — GAS 提供完整的 Ability/Effect/Tag 框架

## Allowed Plugins

- GameplayAbilities (GAS) — 战斗/技能系统
- GameplayTags — 状态管理
- GameplayTasks — 异步任务
- EnhancedInput — 输入系统
- Paper2D — 2D sprite（过渡用，后期换3D模型）
- AIModule — AI/Behavior Tree
- UMG — UI/HUD

## Architecture Decisions Log

- [ADR-001: C++ as Primary Language](../docs/architecture/adr-001-cpp-primary.md) — Accepted
- [ADR-002: GameInstance Subsystems for Cross-Cutting Services](../docs/architecture/adr-002-gi-subsystems.md) — Accepted
- [ADR-003: Signal Bus over Direct References](../docs/architecture/adr-003-signal-bus.md) — Accepted (Delegates)
- [ADR-004: DataAsset-Based Data Configuration](../docs/architecture/adr-004-dataasset-config.md) — Accepted
- [ADR-005: Fixed Room Size](../docs/architecture/adr-005-fixed-room-size.md) — Accepted
- [ADR-006: DeltaTime-Based Combat Timing](../docs/architecture/adr-006-deltatime-combat.md) — Accepted
- [ADR-007: GAS for Combat System](../docs/architecture/adr-007-gas-combat.md) — Accepted

## Engine Specialists

- **Primary**: unreal-specialist
- **Language/Code Specialist**: unreal-cpp-specialist
- **GAS Specialist**: unreal-gas-specialist
- **UI Specialist**: unreal-umg-specialist
- **AI Specialist**: unreal-ai-specialist (Behavior Tree, EQS)

### File Extension Routing

| File Extension / Type | Specialist to Spawn |
|-----------------------|---------------------|
| C++ header (.h) | unreal-cpp-specialist |
| C++ source (.cpp) | unreal-cpp-specialist |
| Build config (.Build.cs) | unreal-cpp-specialist |
| Blueprint (.uasset) | unreal-specialist |
| UI widget (.uasset UMG) | unreal-umg-specialist |
| Behavior Tree (.uasset) | unreal-ai-specialist |
| Data Asset (.uasset) | unreal-specialist |
| Animation Blueprint (.uasset) | unreal-specialist |
