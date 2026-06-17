# Architecture Document — 格斗萌主

> **Version**: 3.0 (3D 转型)
> **Last Updated**: 2026-06-17
> **Status**: Active
> **Engine**: Unreal Engine 5.7
> **Language**: C++ (核心) + Blueprint (内容)

## Architecture Overview

格斗萌主采用**分层架构** + **组件模式**，基于 UE5 Actor-Component 框架。自底向上分为 5 层：Framework → Core → Gameplay → Presentation → Infrastructure。

```
┌─────────────────────────────────────────────┐
│           Infrastructure Layer              │  Subsystems: CombatData, GameManager, SignalBus
├─────────────────────────────────────────────┤
│           Presentation Layer                │  UMG HUD, DungeonFlow, DungeonRoom
├─────────────────────────────────────────────┤
│           Gameplay Layer                    │  GAS Abilities, EnemyAI, ComboSystem, LockOn
├─────────────────────────────────────────────┤
│           Core Layer                        │  PlayerCharacter, CameraController, HitBox/HurtBox
├─────────────────────────────────────────────┤
│           Framework Layer                   │  UE5 Engine, GAS, EnhancedInput
└─────────────────────────────────────────────┘
```

## UE5 Actor-Component Tree

```
World (Level)
├── DungeonFlow (Actor)                    # 关卡流程管理
│   └── DungeonRoom × N (Actor)            # 房间实例
│       ├── Arena Bounds (BoxComponent)    # 竞技场边界
│       └── BaseEnemy × N (Character)      # 敌人
├── PlayerCharacter (Character)            # 玩家角色
│   ├── AbilitySystemComponent             # GAS 技能系统
│   ├── HitBoxComponent                    # 攻击判定 (3D)
│   ├── HurtBoxComponent                   # 受击判定 (3D)
│   └── CharacterMovementComponent         # 3D 自由移动
├── CameraRig (Actor)                      # 第三人称相机组件
│   ├── SpringArm                          # 弹簧臂（碰撞避免）
│   └── Camera                             # 主相机
├── LockOnManager (Actor)                  # 锁定目标管理
└── HUDWidget (UMG)                        # UI层 (Canvas)

Subsystems (GameInstance):
├── USignalBusSubsystem                    # 全局事件
├── UCombatDataSubsystem                   # 战斗配置
└── UGameManagerSubsystem                  # 游戏状态
```

## Core Systems

### 1. Signal Bus (事件总线)

跨系统通信通过 `Dynamic Multicast Delegate` 实现，避免直接引用。

```cpp
// 定义
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHitLanded, 
    AActor*, Attacker, AActor*, Target, float, Damage, FVector, HitLocation);

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
    UPROPERTY() TArray<FAttackMoveData> HeavyAttacks;
    UPROPERTY() float DodgeDistance;
    UPROPERTY() float DodgeSpeed;
    // ...
};
```

### 3. 3D 自由移动

```cpp
// ACharacter + 全3D移动，无轴锁定
MoveComp->bConstrainToPlane = false;  // 解除平面约束
MoveComp->MaxWalkSpeed = 600.f;
MoveComp->JumpZVelocity = 600.f;
MoveComp->AirControl = 0.35f;  // 空中部分控制
```

### 4. 第三人称相机

```cpp
// 相机组件结构
USpringArmComponent* SpringArm;  // 碰撞避免 + 距离控制
UCameraComponent* Camera;        // 主相机

// 相机行为
// - 自由模式: 玩家控制右摇杆旋转
// - 战斗模式: 自动跟随锁定目标
// - 闪避模式: 子弹时间 + 拉近特写
```

### 5. 锁定系统 (Lock-On)

```cpp
UCLASS()
class ULockOnComponent : public UActorComponent
{
    UPROPERTY() AActor* CurrentLockTarget;
    UPROPERTY() float LockRange = 2000.f;
    UPROPERTY() float LockFOV = 120.f;
    
    UFUNCTION() void LockOnNearest();
    UFUNCTION() void SwitchTarget(bool bNext);
    UFUNCTION() void ReleaseLock();
};
```

### 6. GAS 战斗系统

```
UGameplayAbility (基类: UCombatAbility)
    → BP_LightAttack (蓝图子类)
        → AnimMontage (攻击动画)
            → AnimNotify (触发 HitBox 激活)
                → 3D HitBox 碰撞检测
```

## 3D 战斗空间设计

### 竞技场式房间

每个战斗房间是有限3D空间（类似崩坏3的副本房间）：

| 类型 | 大小 | 敌人容量 | 垂直元素 |
|---|---|---|---|
| 小型 (Minion) | 15m × 15m × 8m | 3-5 | 无 |
| 中型 (Elite) | 25m × 25m × 10m | 1-2 精英 | 平台 × 1-2 |
| 大型 (Boss) | 35m × 35m × 15m | 1 Boss | 多层平台 |

### 碰撞系统

| 组件 | 形状 | 用途 |
|---|---|---|
| Player Capsule | Capsule (r=40, h=120) | 角色物理碰撞 |
| HitBox (攻击) | Box/Sphere (动画同步) | 攻击判定 |
| HurtBox (受击) | Capsule (跟随角色) | 受击判定 |
| Arena Bounds | Box (房间边界) | 限制活动范围 |

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
          ┌────┼───────────────────
          ▼    ▼      ▼          ▼
     ┌────────┐ ┌─────┐ ┌──────────┐ ┌────────┐
     │  GAS   │ │Combo│ │ EnemyAI  │ │LockOn  │
     │Ability │ │Mgr  │ │Controller│ │System  │
     └───┬────┘ └─────┘ └──────────┘ └────────┘
         │                   │
    ┌────┼──────┐       ┌────┼────┐
    ▼    ▼      ▼       ▼    ▼    ▼
┌─────┐┌────┐┌─────┐┌─────┐┌────┐┌────┐
│Camera││ HUD││Dun- ││Dun- ││Boss││Loot│
│Rig  ││Wid-││Room ││Flow ││ HP ││    │
│     ││get ││     ││     ││    ││    │
└─────┘└────┘└─────┘└─────┘└────┘└────┘
```

## Key Architectural Decisions

| ADR | 决策 | 状态 |
|-----|------|------|
| [ADR-001](adr-001-cpp-primary.md) | C++ 为主语言 + Blueprint 为内容 | ✅ Accepted |
| [ADR-002](adr-002-gi-subsystems.md) | GameInstanceSubsystem 全局服务 | ✅ Accepted |
| [ADR-003](adr-003-signal-bus.md) | Signal Bus 松耦合通信 | ✅ Accepted |
| [ADR-004](adr-004-dataasset-config.md) | DataAsset 数据驱动 | ✅ Accepted |
| [ADR-005](adr-005-fixed-room-size.md) | 3D 竞技场式关卡空间 | 🔄 Updated |
| [ADR-006](adr-006-deltatime-combat.md) | DeltaTime 战斗时序 | ✅ Accepted |
| [ADR-007](adr-007-gas-combat.md) | GAS 战斗框架 | ✅ Accepted |
| [ADR-008](adr-008-camera-system.md) | 第三人称相机 + 战斗锁定 | 🆕 TODO |
| [ADR-009](adr-009-lock-on.md) | 锁定系统设计 | 🆕 TODO |

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
    │   ├── Camera/             # CameraRig, CameraController [NEW]
    │   ├── Combat/             # HitBox, HurtBox, LockOn [NEW]
    │   ├── Input/              # Enhanced Input config
    │   ├── AI/                 # EnemyAIController (3D NavMesh)
    │   ├── Dungeon/            # DungeonRoom, DungeonFlow
    │   ├── UI/                 # HUDWidget
    │   ── DataAssets/         # CharacterDataAsset
    └── Private/                # Implementations
```

## Performance Budgets

| System | Budget | Target |
|--------|--------|--------|
| Player tick | < 0.5ms | 60fps |
| Enemy tick (×10) | < 3ms | 10 enemies active (3D AI 更复杂) |
| Combat (HitBox check) | < 0.5ms | 3D碰撞更复杂 |
| Camera update | < 0.3ms | SpringArm + LockOn |
| HUD update | < 0.3ms | All bars + combo + lock-on UI |
| Room transition | < 100ms | Level streaming (3D关卡更大) |
| Total game frame | < 12ms | Leave 4ms for render |

## 2.5D → 3D 迁移要点

| 模块 | 2.5D 实现 | 3D 实现 | 影响 |
|---|---|---|---|
| 移动 | Y轴锁定，X轴单轴 | 全3D自由移动 | 重写 PlayerCharacter |
| 相机 | 固定侧视 | 第三人称+锁定 | 新增 CameraRig |
| 碰撞 | 2D BoxComponent | 3D Capsule/Box | 重写 HitBox/HurtBox |
| 房间 | 1920×1080 固定 | 15-35m 3D竞技场 | 重写 DungeonRoom |
| AI | 2D 状态机 | NavMesh 3D寻路 | 重写 EnemyAI |
| 输入 | 左右移动 | WASD+右摇杆 | 重写 InputConfig |
| 战斗 | 单平面判定 | 3D空间锁定+闪避 | 新增 LockOn 系统 |

## Error Handling

| Error Type | Strategy |
|-----------|----------|
| Missing DataAsset | Log + default values |
| Invalid state transition | Log + stay current |
| Null Actor reference | Guard + early return |
| Level load failure | Fallback level |
| Performance spike | UE5 stat monitoring |
| Camera collision | SpringArm 自动拉近 |
| LockOn target lost | 自动切换到最近目标 |
