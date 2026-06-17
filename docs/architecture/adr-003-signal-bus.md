# ADR-003: Signal Bus over Direct References

- **Status**: Accepted
- **Date**: 2026-06-17
- **Engine**: Unreal Engine 5.6

## Context

游戏系统间需要松耦合通信（战斗事件、UI 更新、关卡流程）。直接引用会导致循环依赖和难以维护的代码。

## Decision

**使用 `Dynamic Multicast Delegate` 实现全局事件总线（Signal Bus）。**

### 事件分类

```cpp
USignalBusSubsystem
├── Combat:   OnHitLanded, OnComboFinished, OnComboUpdated, OnHitStopRequested
├── Player:   OnPlayerHealthChanged, OnPlayerEnergyChanged, OnPlayerDied
├── Enemy:    OnEnemyDied
── Dungeon:  OnRoomCleared, OnRoomEntered, OnDungeonCompleted
└── UI:       OnShowDamageNumber
```

### 绑定方式

- **Blueprint**: 直接在蓝图中连线 SignalBus 事件 → 响应函数
- **C++**: 使用 `AddDynamic(this, &UMyClass::MyHandler)` 绑定 UFUNCTION 成员

### 约束

- 事件处理器必须是 `UFUNCTION()` 标记的成员函数
- 不支持 C++ lambda 绑定（`AddDynamic` 限制）
- 事件参数使用 Blueprint 兼容类型（`AActor*`, `float`, `FVector` 等）

## Consequences

- ✅ 系统间零耦合通信
- ✅ 新系统只需订阅事件，无需修改发送方
- ✅ Blueprint 友好（可视化连线）
- ⚠️ 调试时事件流向不够直观（需日志辅助）
- ️ C++ 端不能用 lambda，需要写成员函数
