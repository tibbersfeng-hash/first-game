# Enemy AI (3D 版)

> **Status**: Draft (Updated for 3D)
> **Last Updated**: 2026-06-17
> **Implements Pillar**: 爽快打击+Q版反差 / 职业深度与成长
> **Reference**: 崩坏3、战双、UE5 AI System

## Summary

敌人AI系统管理所有非玩家角色在3D空间中的行为决策、NavMesh寻路、状态机和攻击模式。它提供通用AI框架（用于小怪）和Boss专用行为树（用于Boss），通过数据驱动的配置表定义不同敌人的行为。核心目标是让小怪"够蠢够爽"（让玩家围殴），让Boss"够难够记忆"（需要学习但可战胜）。

> **Quick reference** — Layer: `Feature` · Priority: `MVP` · Key deps: `Combat System, Hit Detection, NavMesh`

## Overview

3D敌我AI的核心差异：
- **NavMesh 寻路**: 在3D空间中导航（不是2D左右移动）
- **包围机制**: 多敌人自动包围玩家（避免堆叠）
- **攻击范围**: 不同敌人有不同3D攻击范围（近战/远程/AOE）
- **垂直空间**: Boss可利用平台、跳跃、飞行
- **群体协作**: 多敌人避免同时攻击，形成节奏

## Enemy Categories (3D)

| Category | HP Ratio | Decision Speed | Attack Slots | Behavior |
|---|---|---|---|---|
| 小怪(Minion) | 1x | 慢(1.0-2.0s) | 不限制 | NavMesh接近→环绕→攻击 |
| 精英(Elite) | 3x | 中(0.5-1.0s) | 不限制 | 小怪行为+特殊技能+护甲 |
| Boss | 10-30x | 快(0.3-0.8s) | 独占 | 阶段切换+多技能组合 |

### 包围算法

```
多个敌人时，自动分配攻击槽位:
- 最多 4 个敌人同时接近玩家（内圈）
- 其他敌人在外圈等待
- 内圈敌人倒下后，外圈补位
- 包围角度：以玩家为圆心，90°间隔分布
```

## NavMesh 寻路

### 寻路规则

| Rule | Detail |
|---|---|
| 使用 | UE5 NavMesh + NavMeshBoundsVolume |
| 房间 | 每个竞技场有独立 NavMesh |
| 动态 | 可破坏物件影响 NavMesh |
| 垂直 | 支持跳跃点(jump link) |
| 避开 | 避开其他敌人（RVO） |

### 接近策略

```
if (DistanceToPlayer > AttackRange) {
    // NavMesh 接近玩家
    MoveTo(PlayerLocation, AcceptanceRadius = 50cm);
    
    // 包围逻辑：计算理想位置
    FVector IdealPos = CalculateFlankingPosition(PlayerLocation, MySlotIndex);
    // 调整目标点
}
```

## Behavior Tree Structure (Boss)

```
Root (Selector)
├── [Phase Check] 
│   ├── Phase 1 (HP > 66%) → Basic Attack Selector
│   ├── Phase 2 (HP > 33%) → Enhanced Attack Selector
│   └── Phase 3 (HP ≤ 33%) → Desperate Attack Selector
│
├── Basic Attack Selector (随机选择)
│   ├── 普通攻击序列 (3连击)
│   ├── 冲撞 (直线突进)
│   └── 远程弹幕 (站立释放)
│
├── Enhanced Attack Selector (Phase 2+)
│   ├── 跳砸 (跳跃+AOE)
│   ├── 旋转攻击 (360° AOE)
│   └── 召唤小怪
│
└── Desperate Attack Selector (Phase 3)
    ├── 狂暴冲撞 (高速连续)
    ├── 全屏AOE (需要躲避提示)
    └── 濒死反击 (被击时触发)
```

## Attack Telegraphing (3D)

所有攻击必须有明确预兆，让玩家有反应时间：

| Attack Type | 预兆时间 | 预兆表现 |
|---|---|---|
| 普通攻击 | 0.5s | 举武器+红色警告圈 |
| 重攻击 | 1.0s | 蓄力特效+地面指示 |
| 冲撞 | 1.5s | 面向玩家+红色直线指示 |
| AOE | 2.0s | 脚下红圈+蓄力动画 |
| 必杀 | 3.0s | 全屏警告+特殊音效 |

### 预兆表现

```
预兆阶段:
- 红色半透明区域显示攻击范围（地面/空中）
- 区域随时间扩大/变化
- 敌人身上蓄力特效
- 警告音效
- 玩家UI显示"闪避!"提示

攻击阶段:
- 预兆消失
- 实际伤害判定
- 命中特效
```

## Interactions with Other Systems

| System | Interaction | Interface |
|---|---|---|
| Combat System | 使用战斗规则 | ApplyDamage(), GetCombatState() |
| Hit Detection | 攻击/受击判定 | HitBox/HurtBox 激活 |
| LockOn System | 被锁定时的表现 | OnLockOnChanged() |
| Dungeon Room | 波次生成 | OnWaveSpawned() |
| NavMesh | 3D寻路 | NavMeshPath |
| Animation | AI驱动动画 | BehaviorTree + AnimMontage |

## Edge Cases (3D)

| Scenario | Expected Behavior |
|---|---|
| 玩家离开房间 | 敌人回到中心点等待 |
| Boss卡地形 | NavMesh 自动绕行，不会穿墙 |
| 多个Boss | 不会同时放必杀（攻击槽位） |
| 玩家被围住 | 内圈4个+外圈等待 |
| Boss击飞玩家 | 玩家着地后起身无敌 |
| 敌人在平台上 | NavMesh 包含平台寻路 |
