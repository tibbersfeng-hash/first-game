# ADR-006: DeltaTime-Based Combat Timing

- **Status**: Accepted
- **Date**: 2026-06-17 (migrated from frame-based)
- **Engine**: Unreal Engine 5.6

## Context

格斗游戏的攻击帧数、HitStop、连招判定等需要精确时间控制。原 Godot 方案使用帧计数（60fps 基准）。UE5 的 Tick 基于 DeltaTime。

## Decision

**以 DeltaTime（秒）为基础，内部按 60fps 帧数换算。**

### 换算公式

```cpp
float FrameToSeconds(float Frames) { return Frames / 60.f; }
float SecondsToFrame(float Seconds) { return Seconds * 60.f; }
```

### 应用场景

| 系统 | 实现方式 |
|------|---------|
| 攻击帧 (Startup/Active/Recovery) | `FTimerManager::SetTimer` + 秒数 |
| HitStop | Tick 中累加 DeltaTime，达到阈值后解除 |
| 连招窗口 | Tick 中倒计时，归零重置 |
| 能量恢复 | `EnergyRegenRate * DeltaTime` |

### HitStop 实现

```cpp
// Tick 中
if (bIsInHitStop) {
    HitStopTimer -= DeltaTime;
    if (HitStopTimer <= 0.f) bIsInHitStop = false;
    return; // 跳过所有游戏逻辑
}
```

## Consequences

- ✅ 帧率无关（120fps 玩家体验一致）
- ✅ 使用 UE5 原生 DeltaTime 机制
- ✅ 策划仍可按"帧数"思考（60fps 基准）
- ⚠️ 低帧率下 Timer 精度略降（可接受）
- ⚠️ 不支持真正的"逐帧"回放（格斗游戏通常不需要）
