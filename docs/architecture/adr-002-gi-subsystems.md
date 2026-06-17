# ADR-002: GameInstance Subsystems for Cross-Cutting Services

- **Status**: Accepted
- **Date**: 2026-06-17
- **Engine**: Unreal Engine 5.6

## Context

游戏需要多个全局单例服务（信号总线、战斗数据、游戏状态管理），在 Godot 中使用 Autoload 模式。UE5 中需要等价方案。

## Decision

**使用 `UGameInstanceSubsystem` 实现全局单例服务。**

### 架构

```
UGameInstance
├── USignalBusSubsystem        — 全局事件广播
├── UCombatDataSubsystem       — 战斗数据配置（帧率、连招上限、HitStop时长）
└── UGameManagerSubsystem      — 游戏状态机（Menu/Playing/Paused/GameOver）
```

### 注册方式

子系统通过 `UCLASS()` + `UGameInstanceSubsystem` 基类自动注册。无需手动创建，通过 `GetSubsystem<T>()` 访问。

### 访问方式

```cpp
// C++
USignalBusSubsystem* SignalBus = GetGameInstance()->GetSubsystem<USignalBusSubsystem>();

// Blueprint (via FunctionLibrary)
USignalBusSubsystem* SignalBus = USignalBusFunctionLibrary::GetSignalBus(this);
```

## Consequences

- ✅ 自动生命周期管理（随 GameInstance 创建/销毁）
- ✅ 类型安全访问（模板方法）
- ✅ 蓝图可通过 FunctionLibrary 访问
- ✅ 子系统间解耦
- ⚠️ 不能在游戏运行时动态添加/移除子系统
