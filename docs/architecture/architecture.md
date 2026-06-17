# Architecture Document — 格斗萌主

> **Version**: 2.0 (UE5 Migration)
> **Last Updated**: 2026-06-17
> **Status**: Active
> **Engine**: Unreal Engine 5.6
> **Language**: C++ (核心) + Blueprint (内容)

## Architecture Overview

格斗萌主采用**分层架构** + **组件模式**，基于 UE5 Actor-Component 框架。自底向上分为 5 层：Framework → Core → Gameplay → Presentation → Infrastructure。

```
┌─────────────────────────────────────────────┐
│           Infrastructure Layer              │  Subsystems: CombatData, GameManager, SignalBus
├─────────────────────────────────────────────┤
│           Presentation Layer                │  UMG HUD, DungeonFlow, DungeonRoom
├─────────────────────────────────────────────┤
│           Gameplay Layer                    │  GAS Abilities, EnemyAI, ComboSystem
├─────────────────────────────────────────────┤
│           Core Layer                        │  PlayerCharacter, HitBox/HurtBox, InputConfig
├─────────────────────────────────────────────┤
│           Framework Layer                   │  UE5 Engine, GAS, EnhancedInput, Paper2D
└─────────────────────────────────────────────┘
```

## UE5 Actor-Component Tree

```
World (Level)
├── DungeonFlow (Actor)                # 关卡流程管理
│   └── DungeonRoom × N (Actor)        # 房间实例
│       └── BaseEnemy × N (Character)  # 敌人
├── PlayerCharacter (Character)        # 玩家角色
│   ├── AbilitySystemComponent         # GAS 技能系统
│   ├── HitBoxComponent                # 攻击判定
│   ├── HurtBoxComponent               # 受击判定
│   └── CharacterMovementComponent     # 2D约束移动
├── SideViewCamera (CameraActor)       # 固定侧视相机
└── HUDWidget (UMG)                    # UI层 (Canvas)

Subsystems (GameInstance):
├── USignalBusSubsystem                # 全局事件
├── UCombatDataSubsystem               # 战斗配置
└── UGameManagerSubsystem              # 游戏状态
```

## Core Systems

### 1. Signal Bus (事件总线)

跨系统通信通过 `Dynamic Multicast Delegate` 实现，避免直接引用。

```cpp
// 定义
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHitLanded, AActor*, Attacker, AActor*, Target, float, Damage, FVector, HitLocation);

// 广播
SignalBus->OnHitLanded.Broadcast(Attacker, Target, Damage, Location);

// 绑定 (Blueprint)
// 直接在蓝图中连线
```

### 2. DataAsset 配置

所有游戏数值从 `UPrimaryDataAsset` 加载，不硬编码。

```cpp
UCLASS()
class UCharacterDataAsset : public UPrimaryDataAsset
{
    UPROPERTY() float MaxHealth;
    UPROPERTY() TArray<FAttackMoveData> LightAttacks;
    // ...
};
```

### 3. 2D 平面约束移动

```cpp
// ACharacter + 锁定Y轴
MoveComp->bConstrainToPlane = true;
MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
```

### 4. GAS 战斗系统

```
UGameplayAbility (基类: UCombatAbility)
    → BP_LightAttack (蓝图子类)
        → AnimMontage (攻击动画)
            → AnimNotify (触发 HitBox 激活)
```

## Module Dependency

```
          ┌──────────────┐
          │ CombatData   │ (Subsystem)
          └──────┬───────┘
                 │ (config)
    ┌────────────┼────────────┐
    ▼            ▼            ▼
┌──────┐  ┌──────────┐  ┌──────────┐
│Input │──▶│  Player  │◀─▶│ HitBox/  │
│Config│  │Character │  │ HurtBox  │
└──────┘  └────┬─────┘  └──────────┘
          ┌────┼────
          ▼    ▼    ▼
     ┌────────┐ ┌─────┐ ┌──────────┐
     │  GAS   │ │Combo│ │ EnemyAI  │
     │Ability │ │Mgr  │ │Controller│
     └───┬────┘ └─────┘ └─────────┘
         │                   │
    ┌────┼──────┐       ┌────┼────┐
    ▼    ▼      ▼       ▼    ▼    ▼
┌─────┐────┐┌─────┐┌────┐┌────┐┌────
│ HUD ││Dun-││Dun- ││Dmg ││Boss││Loot│
│Widget││Room││Flow ││Num ││ HP ││    │
└─────┘└────┘└─────┘└────┘└────┘└────┘
```

## Key Architectural Decisions

| ADR | 决策 | 状态 |
|-----|------|------|
| [ADR-001](adr-001-cpp-primary.md) | C++ 为主语言 + Blueprint 为内容 | ✅ Accepted |
| [ADR-002](adr-002-gi-subsystems.md) | GameInstanceSubsystem 全局服务 | ✅ Accepted |
| [ADR-003](adr-003-signal-bus.md) | Signal Bus 松耦合通信 | ✅ Accepted |
| [ADR-004](adr-004-dataasset-config.md) | DataAsset 数据驱动 | ✅ Accepted |
| [ADR-005](adr-005-fixed-room-size.md) | 固定房间尺寸 | ✅ Accepted |
| [ADR-006](adr-006-deltatime-combat.md) | DeltaTime 战斗时序 | ✅ Accepted |
| [ADR-007](adr-007-gas-combat.md) | GAS 战斗框架 | ✅ Accepted |

## Directory Structure

```
src_ue5/
├── FirstGame.uproject
├── Config/
│   ├── DefaultGame.ini
│   ├── DefaultEngine.ini
│   └── DefaultInput.ini
├── Content/                    # Assets (Blueprint, meshes, textures)
└── Source/FirstGame/
    ├── FirstGame.Build.cs
    ├── Public/                 # Headers
    │   ├── Subsystems/         # SignalBus, CombatData, GameManager
    │   ├── Characters/         # PlayerCharacter, BaseEnemy
    │   ├── Combat/             # HitBox, HurtBox
    │   ├── Input/              # Enhanced Input config
    │   ├── AI/                 # EnemyAIController
    │   ├── Dungeon/            # DungeonRoom, DungeonFlow
    │   ├── UI/                 # HUDWidget
    │   ── DataAssets/         # CharacterDataAsset
    └── Private/                # Implementations
```

## Performance Budgets

| System | Budget | Target |
|--------|--------|--------|
| Player tick | < 0.5ms | 60fps |
| Enemy tick (×10) | < 2ms | 10 enemies active |
| Combat (HitBox check) | < 0.3ms | Per frame |
| HUD update | < 0.3ms | All bars + combo |
| Room transition | < 50ms | Level streaming |
| Total game frame | < 12ms | Leave 4ms for render |

## Error Handling

| Error Type | Strategy |
|-----------|----------|
| Missing DataAsset | Log + default values |
| Invalid state transition | Log + stay current |
| Null Actor reference | Guard + early return |
| Level load failure | Fallback level |
| Performance spike | UE5 stat monitoring |
