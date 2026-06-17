# ADR-005: Fixed Room Size

- **Status**: Accepted
- **Date**: 2026-06-15 (updated 2026-06-17 for UE5)
- **Engine**: Unreal Engine 5.6

## Context

2.5D 横版格斗闯关游戏，每个战斗房间需要固定尺寸以保证战斗体验一致性。

## Decision

**每个房间固定为 1920×1080 游戏世界单位（16:9 比例）。**

### UE5 实现

- 房间大小通过 `ADungeonRoom` Actor 的 BoxComponent 定义
- 摄像机固定在房间中心，正交或透视模式
- 角色移动限制在 XZ 平面（Y 轴锁定）
- 房间边界用 Invisible Wall 或硬编码坐标限制

### 房间类型

| 类型 | 用途 | 敌人数 |
|------|------|--------|
| Combat | 普通战斗 | 2-4 |
| Treasure | 宝箱/奖励 | 0 |
| Boss | Boss 战 | 1 (大体型) |
| Rest | 恢复/商店 | 0 |

## Consequences

- ✅ 战斗体验一致，镜头控制简单
- ✅ 关卡设计可预测
- ✅ 性能可控（固定可见范围）
- ⚠️ 不支持动态大小房间（如 Boss 超大场地需特殊处理）
