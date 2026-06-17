# Control Manifest — UE5

> **Engine**: Unreal Engine 5.6
> **Last Updated**: 2026-06-17
> **ADRs Covered**: ADR-001 ~ ADR-007
> **Status**: Active

---

## Foundation Layer Rules

### Required Patterns

- **C++ for core systems, Blueprint for content** — 性能关键用 C++，关卡/动画/UI 用 BP — source: ADR-001
- **GameInstanceSubsystem for global services** — SignalBus, CombatData, GameManager — source: ADR-002
- **CombatData is read-only at runtime** — 运行时不修改 DataAsset 属性 — source: ADR-002
- **GameManager is sole writer of game state** — 只有 GameManager 可修改游戏状态 — source: ADR-002
- **SignalBus is stateless** — 只声明和广播事件，不持有游戏状态 — source: ADR-002
- **SignalBus for all cross-layer communication** — 跨层通信必须通过 SignalBus — source: ADR-003
- **Signal handlers must be UFUNCTION** — AddDynamic 需要 UFUNCTION 成员函数，不支持 lambda — source: ADR-003
- **All config data uses UPrimaryDataAsset** — 不用 JSON/CSV，用 DataAsset — source: ADR-004
- **Every DataAsset has GetPrimaryAssetId()** — 支持 AssetManager 加载 — source: ADR-004
- **DataAssets stored in Content/DataAssets/** — 按类型分子目录 — source: ADR-004

### Forbidden Approaches

- **Never use Blueprint for combat logic** — C++ 保证帧精确 — source: ADR-001
- **Never allow Subsystems to reference each other directly** — 通过 SignalBus 通信 — source: ADR-002
- **Never use direct Actor references for cross-layer communication** — 用 SignalBus 解耦 — source: ADR-003
- **Never use AddDynamic with lambdas** — 编译错误，必须用 UFUNCTION 成员 — source: ADR-003
- **Never hardcode game values** — 必须从 DataAsset 加载 — source: ADR-004
- **Never mutate DataAsset at runtime** — 运行时状态用独立变量 — source: ADR-004

---

## Core Layer Rules

### Required Patterns

- **DeltaTime-based combat timing (60fps基准)** — 帧数换算为秒: `Frames / 60.f` — source: ADR-006
- **2D plane constraint on CharacterMovement** — `SetPlaneConstraintAxisSetting(Y)` — source: ADR-005
- **Room size 1920×1080 world units** — 固定尺寸，无镜头滚动 — source: ADR-005
- **Side-view camera fixed at room center** — 正交或透视模式 — source: ADR-005
- **HitBox/HurtBox use custom collision channels** — GameTraceChannel1/2 — source: ADR-003

### Forbidden Approaches

- **Never use Paper2D for main characters** — 2.5D 用 3D Character + 固定相机 — source: ADR-001
- **Never use delta accumulation for combat timing** — 用 Timer 或帧换算 — source: ADR-006
- **Never use Camera follow/scroll** — 固定一屏战斗 — source: ADR-005

---

## Feature Layer Rules

### Required Patterns

- **GAS for all combat abilities** — 轻攻/重攻/必杀都继承 UCombatAbility — source: ADR-007
- **GameplayTag for state management** — Status.Attacking, Status.HitStun, Status.Dead — source: ADR-007
- **Behavior Tree for enemy AI** — Detect → Chase → Attack 三阶段 — source: ADR-007
- **Enemy data from DataAsset** — 每个敌人类型一个 DataAsset — source: ADR-004
- **Combo tracking via ComboManager component** — 窗口期 + 计数 + 重置 — source: ADR-007
- **HitStop via WorldSettings TimeDilation** — 全局减速 + 组件独立 Tick — source: ADR-006

### Forbidden Approaches

- **Never write to another system's state directly** — 用 SignalBus — source: ADR-003
- **Never implement custom skill system alongside GAS** — 统一用 GAS — source: ADR-007
- **Never mix frame-based and delta-based timing** — 统一用 DeltaTime + 帧换算 — source: ADR-006

---

## Presentation Layer Rules

### Required Patterns

- **HUD via UMG Widget** — 绑定到 PlayerController — source: ADR-001
- **HUD updates are signal-driven** — 监听 SignalBus 事件 — source: ADR-003
- **Damage numbers as separate Widget** — 世界位置 + 动画 — source: ADR-003

### Forbidden Approaches

- **Never reference combat systems directly from HUD** — 只消费 SignalBus — source: ADR-003
- **Never manipulate UI from Character scripts** — 用信号解耦 — source: ADR-003

---

## Global Rules

### Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes | Prefix + PascalCase | `APlayerCharacter`, `UHitBoxComponent` |
| Variables | camelCase | `currentHealth`, `baseDamage` |
| Delegates | F prefix | `FOnHitLanded`, `FOnComboFinished` |
| Enums | E prefix | `EGameState`, `ERoomType` |
| Structs | F prefix | `FAttackMoveData`, `FDamageNumberData` |
| Files | PascalCase | `PlayerCharacter.h`, `HitBoxComponent.cpp` |
| Blueprint assets | BP_ prefix | `BP_LightAttack`, `BP_EnemyGoblin` |
| DataAssets | DA_ prefix | `DA_Huikong`, `DA_Goblin` |

### Performance Budgets

| Target | Value |
|--------|-------|
| Framerate | 60 FPS (战斗场景必须稳定) |
| Player tick | < 0.5ms |
| Enemy tick (×10) | < 2ms |
| Combat (HitBox) | < 0.3ms |
| HUD update | < 0.3ms |
| Room transition | < 50ms |
| Total game frame | < 12ms |

### Approved Plugins

- GameplayAbilities (GAS)
- GameplayTags
- GameplayTasks
- EnhancedInput
- Paper2D（过渡用）
- AIModule
- UMG
