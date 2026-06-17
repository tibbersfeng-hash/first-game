# Combat System (3D 版)

> **Status**: Draft (Updated for 3D)
> **Last Updated**: 2026-06-17
> **Implements Pillar**: 爽快打击+Q版反差 / 职业深度与成长
> **Reference**: 崩坏3、战双帕弥什、绝区零

## Summary

战斗系统定义了格斗萌主的核心战斗规则——3D空间中的攻击类型、动作取消、能量管理、闪避反击、锁定切换和战斗状态流转。它是连招手感的骨架：Input System 提供输入，Player Controller 提供身体，Camera/LockOn 提供视角，Hit Detection 提供反馈，而 Combat System 决定"出什么招、能取消什么、消耗多少、怎么连"。

> **Quick reference** — Layer: `Feature` · Priority: `MVP` · Key deps: `Player Controller, Hit Detection, LockOn System`

## Overview

3D战斗系统的核心差异：
- **锁定视角**: 战斗时始终面向锁定目标，移动相对目标环绕
- **全向闪避**: 可向前/后/左/右闪避，相对目标方向
- **技能方向性**: 部分技能有方向/范围（前方扇形、圆形AOE等）
- **空中连招**: 3D空间空中攻击可调整方向
- **必杀演出**: 触发必杀时镜头拉近特写

## Player Fantasy

战斗像跳舞——轻攻是快步，重攻是重踏，必杀是旋转跳跃。3D空间让你可以绕到敌人背后输出，闪避到侧面反击。每次出招都是承诺（投入时间和能量），每次取消都是救赎（用更贵的动作换掉当前动作）。好玩家打得漂亮（空间利用+时机精准），菜玩家也能打得爽（3键连招足够）。

## Detailed Design

### Core Rules

1. **攻击类型**: 轻攻(快/低伤)、重攻(慢/中伤)、技能(中速/特效)、必杀(最慢/高伤/高耗能量)
2. **动作取消**: 特定动作可在recovery帧被更高优先级动作取消
3. **能量系统**: 攻击命中积攒能量，必杀消耗能量，闪避消耗体力（不消耗能量）
4. **防御**: 无格挡机制，闪避是唯一防御手段
5. **无敌帧**: 仅闪避和起身时有无敌帧
6. **空中战斗**: 可空中攻击，但只有1次空中攻击机会（着地重置）
7. **锁定影响**: 战斗中锁定目标会影响攻击方向、移动方式、闪避方向

### Attack Type Definitions (3D)

| Type | Startup | Active | Recovery | Damage | Energy Cost | Cancel Priority | HitBox Shape |
|---|---|---|---|---|---|---|---|
| Light 1 | 3f | 4f | 5f | 10 | 0 | 1 | 前方扇形 |
| Light 2 | 3f | 4f | 5f | 12 | 0 | 1 | 前方扇形 |
| Light 3 | 4f | 5f | 6f | 15 | 0 | 1 | 前方扇形（大） |
| Heavy | 8f | 5f | 10f | 25 | 0 | 2 | 前方矩形 |
| Skill 1 | 6f | 6f | 8f | 20 | CD:3s | 2 | 按技能定义 |
| Skill 2 | 8f | 6f | 10f | 30 | CD:8s | 2 | 按技能定义 |
| Ultimate | 12f | 10f | 20f | 60 | 30 | 3 | 大范围（镜头演出） |
| Air Light | 4f | 4f | 7f | 8 | 0 | 1 | 下方扇形 |
| Air Heavy | 8f | 5f | 12f | 20 | 0 | 2 | 下方圆形 |

*注: 3D版本的 startup 帧数比2D略长（动作更复杂），但仍在可接受范围*

### 3D HitBox 形状

| Shape | 用途 | 参数 |
|---|---|---|
| 前方扇形 | 轻攻、前方技能 | 角度(60-120°), 距离(1-3m) |
| 前方矩形 | 重攻、突进技能 | 宽×长×高 |
| 圆形AOE | 范围技能、必杀 | 半径(2-8m) |
| 自身碰撞 | 贴身攻击、反击 | Capsule |
| 追踪弹 | 魔导师技能 | 速度、追踪率、寿命 |

### Cancel Rules

| Current Action | Can Cancel Into | Condition |
|---|---|---|
| Light 1 recovery | Light 2 / Heavy / Skill_1 / Dodge | 输入在cancel窗口内 |
| Light 2 recovery | Light 3 / Heavy / Skill_1 / Dodge | 同上 |
| Light 3 recovery | Heavy / Skill_* / Dodge | 不可再接轻攻 |
| Heavy recovery | Skill_* / Dodge | 不可取消回轻攻 |
| Skill recovery | Dodge only | 技能收招长 |
| Ultimate recovery | Dodge only | 必杀收招最长 |
| Air any recovery | Nothing | 空中攻击不可取消 |
| Dodge recovery | Any attack | 闪避后可接任何攻击 |
| HitStun | Nothing | 硬直不可取消 |

**Cancel Window**: recovery帧的前50%是可取消窗口

### 3D 闪避机制

| Dodge Direction | Input | 位移方向 | 距离 |
|---|---|---|---|
| 前闪 | W+Space (或前+闪避) | 相对目标远离 | 300cm |
| 后闪 | S+Space | 相对目标靠近 | 300cm |
| 左闪 | A+Space | 相对目标左环绕 | 300cm |
| 右闪 | D+Space | 相对目标右环绕 | 300cm |
| 原地闪 | 仅Space | 原地旋转（无敌帧） | 0 |

**极限闪避** (Just Dodge):
- 敌人攻击 active 帧内闪避触发
- 效果: 0.5秒子弹时间 + 镜头拉近 + 额外能量+10
- 触发音效特殊提示

### 3D 技能设计 (以职业区分)

| 职业 | Skill 1 | Skill 2 | Ultimate |
|---|---|---|---|
| 武僧 | 气功波（前方直线弹） | 升龙拳（击飞） | 如来神掌（前方扇形大爆发） |
| 魔导师 | 糖果弹（追踪弹） | 棉花糖地雷（放置） | 彩虹风暴（大范围AOE） |
| 狂战士 | 鬼步冲锋（突进） | 震地波（圆形AOE） | 鬼王降临（大范围+击飞） |

### Energy System

| Rule | Detail |
|---|---|
| 初始能量 | 0 |
| 最大能量 | 100 |
| 必杀可用条件 | energy ≥ 30 |
| 必杀消耗 | 30 |
| 能量获取(命中轻攻) | +5 |
| 能量获取(命中重攻) | +12 |
| 能量获取(命中技能) | +8 |
| 能量获取(被击) | +3 |
| 能量获取(极限闪避) | +10 |
| 体力消耗(闪避) | 15 |
| 体力最大 | 100 |
| 体力恢复/秒 | 30 |

### States and Transitions (3D)

| State | Entry | Exit | Movement | Can Attack | Can Dodge |
|---|---|---|---|---|---|
| CombatIdle | 战斗中无动作 | 攻击/闪避/被击 | 环绕目标 | Yes | Yes |
| AttackStartup | 攻击输入 | 进入Active | 面向目标 | No | No |
| AttackActive | Startup结束 | 进入Recovery | 攻击位移 | No | No |
| AttackRecovery | Active结束 | Cancel或自然结束 | 渐恢复 | Cancel only | Cancel only |
| DodgeActive | 闪避输入 | 位移完成 | 闪避方向 | No | No |
| HitStun | 被击中 | 硬直结束 | 击退 | No | No |
| Knockdown | 击倒(HP≤0) | 起身完成 | No | No | 起身后无敌 |
| UltimateCasting | 必杀输入 | 演出结束 | 锁定镜头 | No | No |

### Ground vs Air (3D)

| Aspect | Ground | Air |
|---|---|---|
| 可用攻击 | 全部 | Air Light / Air Heavy |
| 空中攻击次数 | — | 1次/跳跃 |
| 移动 | 3D全向 | 攻击推力方向 |
| 取消 | 按cancel规则 | 不可取消 |
| 着地 | — | 中断攻击→Idle |
| 锁定 | 完整锁定 | 锁定但无环绕 |

## Interactions with Other Systems

| System | Interaction | Interface |
|---|---|---|
| Input System | 消费攻击/闪避输入 | IsActionJustPressed() |
| Player Controller | 驱动战斗状态 | SetCombatState() |
| Hit Detection | 命中时提供伤害参数 | OnHitConfirmed() |
| LockOn System | 提供锁定目标方向 | GetLockTarget() |
| Camera System | 触发镜头演出 | TriggerCameraEffect() |
| Combo System | 连招链驱动攻击序列 | TryComboAdvance() |
| Enemy AI | 敌人查询玩家战斗状态 | GetCombatState() |
| HUD | 显示能量条/技能状态 | OnEnergyChanged() |

## Formulas

### 3D HitBox 命中判定

```cpp
// 扇形判定
bool CheckConeHit(FVector AttackerLoc, FVector Forward, float ConeAngle, float Range, AActor* Target) {
    FVector ToTarget = Target->GetActorLocation() - AttackerLoc;
    float Distance = ToTarget.Size();
    if (Distance > Range) return false;
    
    float Angle = FMath::RadiansToDegrees(FMath::Acos(
        FVector::DotProduct(Forward.GetSafeNormal(), ToTarget.GetSafeNormal())
    ));
    return Angle <= ConeAngle * 0.5f;
}

// 圆形AOE判定
bool CheckSphereHit(FVector Center, float Radius, AActor* Target) {
    return (Target->GetActorLocation() - Center).Size() <= Radius;
}
```

### Damage with Combo Scaling

```cpp
final_damage = floor(base_damage * (1.0 + combo_count * 0.05) * crit_multiplier);
```

### Cancel Window

```cpp
cancel_window_end = recovery_start + floor(recovery_frames * cancel_ratio);
// cancel_ratio = 0.5 (前50%可取消)
```

## Edge Cases (3D)

| Scenario | Expected Behavior | Rationale |
|---|---|---|
| 锁定目标在身后 | 角色转身面向目标 | 保持面向锁定目标 |
| 闪避时目标移动 | 闪避方向基于输入瞬间的目标位置 | 防止意外位移 |
| 多目标重叠 | 优先攻击锁定目标 | 避免误伤 |
| 必杀期间目标死亡 | 必杀继续，伤害浪费 | 必杀高风险高成本 |
| 闪避撞墙 | 闪避停止在墙边 | 隐形墙阻挡 |
| 空中被击中 | 进入空中HitStun，着地后额外硬直 | 空中受击更危险 |
| 技能打空 | 技能正常释放（无命中） | 避免"取消技能"漏洞 |
| 锁定切换中攻击 | 攻击朝原目标方向 | 避免误操作 |

## Tuning Knobs

| Parameter | Default | Safe Range | Effect |
|---|---|---|---|
| max_energy | 100 | 50-200 | 必杀频率 |
| ultimate_cost | 30 | 20-50 | 必杀门槛 |
| dodge_stamina | 15 | 10-25 | 闪避频率 |
| cancel_ratio | 0.5 | 0.3-0.8 | 取消窗口 |
| combo_scale_per_hit | 0.05 | 0.02-0.1 | 连招伤害衰减 |
| i_frames_on_wake | 3 | 0-10 | 起身无敌 |
| air_attack_count | 1 | 0-3 | 空中战斗深度 |
| just_dodge_window | 150ms | 100-250ms | 极限闪避窗口 |
| just_dodge_duration | 0.5s | 0.3-1.0s | 子弹时间长度 |

## Feel Acceptance Criteria (3D)

- [ ] 轻攻到轻攻的取消流畅（无卡顿感）
- [ ] 闪避在150ms内完全无敌
- [ ] 极限闪避触发生明显（子弹时间+镜头拉近+音效提示）
- [ ] 锁定切换平滑（镜头旋转不突兀）
- [ ] 必杀触发时镜头演出自然（拉近→特写→爆发→恢复）
- [ ] 3D空间攻击命中判定准确（扇形/圆形/矩形正确）
- [ ] 空中攻击着地正确中断
- [ ] 起身无敌帧有效（不被压制）
