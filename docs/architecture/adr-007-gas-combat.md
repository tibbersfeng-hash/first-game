# ADR-007: GAS for Combat System

- **Status**: Accepted
- **Date**: 2026-06-17
- **Engine**: Unreal Engine 5.6

## Context

格斗游戏需要复杂的战斗系统：连招、Buff/Debuff、技能冷却、状态管理。需要决定是使用 UE5 的 Gameplay Ability System (GAS) 还是自定义战斗框架。

## Decision

**集成 Gameplay Ability System (GAS) 作为战斗系统核心框架。**

### 架构映射

| 格斗游戏概念 | GAS 组件 |
|-------------|---------|
| 攻击动作 (轻/重/必杀) | `UGameplayAbility` |
| 伤害/治疗/击退 | `UGameplayEffect` |
| 状态 (攻击中/眩晕/无敌) | `GameplayTag` |
| 连招计数器 | `GameplayTag` + Ability 逻辑 |
| 技能冷却 | `UGameplayAbility::Cooldown` |
| 能量消耗 | `UGameplayEffect` (Cost) |
| 属性 (HP/能量/攻击) | `AttributeSet` |

### 技能定义流程

```
C++ 基类 (UCombatAbility)
    → Blueprint 子类 (BP_LightAttack_Huikong)
        → 配置: Damage, StartupFrames, AnimationMontage
        → AnimNotify 触发 HitBox 激活
```

### 状态管理

```cpp
// GameplayTag 层级
Status.Attacking.Light1
Status.Attacking.Light2
Status.Attacking.Heavy
Status.Attacking.Special
Status.Dodging
Status.HitStun
Status.Dead
```

### 不使用 GAS 的部分

- HitBox/HurtBox 碰撞检测 → 自定义 Component（更简单直接）
- 连招窗口计时 → 自定义逻辑（GAS 的 cooldown 不完全匹配）
- HitStop/HitFreeze → 自定义 Tick 逻辑

## Consequences

- ✅ GAS 提供完整的技能/Buff/属性框架
- ✅ 社区支持好，文档丰富
- ✅ 易于扩展新技能和效果
- ✅ 属性系统支持网络同步（未来多人模式）
- ⚠️ GAS 学习曲线陡峭
- ⚠️ 对于简单格斗游戏可能过度设计（但提供了扩展空间）
- ⚠️ AttributeSet 需要额外定义和维护
