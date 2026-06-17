# Dungeon Room System (3D 版)

> **Status**: Draft (Updated for 3D)
> **Last Updated**: 2026-06-17
> **Implements Pillar**: 爽快打击+Q版反差 / 职业深度与成长
> **Reference**: 崩坏3、战双、绝区零

## Summary

副本房间系统管理单个3D竞技场房间的布局、敌人波次、边界锁定和清理条件。它是"清怪→开门→下一间"循环的实现者——玩家进入3D空间，门锁上，清完敌人，门打开，奖励出现。

> **Quick reference** — Layer: `Presentation` · Priority: `MVP` · Key deps: `Enemy AI, ADR-005`

## Overview

Dungeon Room 定义了"一个房间"的一切：
- **3D布局**: 物理空间（平台/墙壁/装饰），不是2D像素布局
- **竞技场边界**: 隐形墙限制活动范围
- **敌人波次**: 1-3波敌人，每波清除后触发下一波
- **清理条件**: 当前波所有敌人HP≤0
- **奖励**: 房间完成后的掉落和评分

## Room Specifications (3D)

### 房间尺寸

| 类型 | 大小 | 敌人容量 | 垂直元素 |
|---|---|---|---|
| 小型 Combat | 15m × 15m × 8m | 3-5 | 无或1个低平台 |
| 中型 Elite | 25m × 25m × 10m | 1-2 精英 | 2个平台 + 高低差 |
| 大型 Boss | 35m × 35m × 15m | 1 Boss | 多层平台 + 可破坏柱子 |
| Treasure | 10m × 10m × 6m | 0 | 宝箱台 + 装饰 |
| Rest | 12m × 12m × 6m | 0 | 恢复点 + 商店NPC |

### Room Layout Components (3D)

| Component | Purpose | Config |
|---|---|---|
| 地面 (Floor) | 角色站立/移动，带NavMesh | 单层或多层（高度差） |
| 平台 (Platform) | 空中战斗/跳跃路线 | 0-4个，位置/大小可配 |
| 墙壁 (Wall) | 边界+视觉装饰 | 隐形墙+视觉墙 |
| 天花板 (Ceiling) | 上边界+跳跃限制 | 固定，不可破坏 |
| 门 (Door) | 进入/离开房间 | 入口+出口（对面） |
| 装饰物 (Props) | 氛围+可破坏物 | 0-10个，可掉落金币 |
| 陷阱 (Trap) | 环境伤害 | 地面尖刺/落石等 |

### Arena Bounds (边界系统)

```cpp
UCLASS()
class ADungeonRoom : public AActor
{
    // 隐形墙边界
    UPROPERTY() UBoxComponent* ArenaBounds;
    
    // 视觉提示（可选）
    UPROPERTY() TArray<AActor*> VisualBoundaries;
    
    // 边界行为
    UPROPERTY() bool bShowBoundaryEffect = true;  // 接近边界时显示能量墙
    UPROPERTY() float BoundaryWarningDistance = 200.f;  // 距离边界2m时警告
};
```

边界处理规则:
- 玩家/敌人接近边界时，隐形墙阻挡
- 视觉提示：接近时显示能量墙效果
- 攻击不会穿过边界（HitBox被边界截断）
- 闪避到边界会被阻挡（不会穿过）

## Wave Configuration (3D)

```cpp
USTRUCT()
struct FWaveConfig
{
    // 敌人类别和数量
    UPROPERTY() TArray<FEnemySpawnEntry> Enemies;
    
    // 出生方式
    UPROPERTY() ESpawnType SpawnType;  // 同时/依次/从传送门
    
    // 波间间隔
    UPROPERTY() float TimeBetweenWaves = 2.0f;
    
    // 特殊规则
    UPROPERTY() bool bIsBossWave = false;
    UPROPERTY() bool bHasElite = false;
};

USTRUCT()
struct FEnemySpawnEntry
{
    UPROPERTY() TSubclassOf<ABaseEnemy> EnemyClass;
    UPROPERTY() int Count = 1;
    UPROPERTY() FVector SpawnOffset;  // 相对出生点偏移
    UPROPERTY() float SpawnDelay = 0.0f;  // 依次生成延迟
};
```

## Room Flow (3D)

```
玩家进入 TriggerVolume
    → 门关闭动画（能量屏障）
    → ArenaBounds 激活
    → 当前波敌人 Spawn (NavMesh 寻路激活)
    → 玩家清完当前波
    → 2秒安全窗口（恢复时间）
    → 下一波 Spawn 或
    → 门开启动画 + 奖励生成 + 评分显示
```

## Camera in Dungeon (3D)

房间对相机的影响:
- **小房间**: 相机自动拉近（避免穿墙）
- **Boss房**: 特定角度触发镜头演出（Boss登场）
- **平台跳跃**: 相机稍微抬高（俯瞰视角）
- **狭窄通道**: 相机贴紧角色（避免穿墙）

## Visual Design (3D)

### 房间氛围

| 房间类型 | 光线 | 色调 | 音效 |
|---|---|---|---|
| Combat | 昏暗+高亮特效 | 深渊底色 #1A1A2E | 紧张背景乐 |
| Elite | 略亮+聚焦光 | 暗紫+金属光 | 更紧张 |
| Boss | 特殊光效+体积光 | 主题色+危险红 | Boss主题乐 |
| Treasure | 柔和+聚焦光 | 暖色+金光 | 发现音效 |
| Rest | 明亮温暖 | 暖棕+柔黄 | 轻松音乐 |

## Performance Budget (3D)

| Element | Budget |
|---|---|
| NavMesh 计算 | < 2ms per frame |
| 敌人AI (×10) | < 3ms per frame |
| 房间静态网格 | < 5ms render |
| 边界检测 | < 0.1ms per frame |
| 房间切换 | < 100ms streaming |

## Acceptance Criteria (3D)

- [ ] 房间边界正确阻挡玩家/敌人
- [ ] 波次系统正确切换（含波间安全窗口）
- [ ] 门动画与边界系统同步
- [ ] NavMesh 在所有房间类型正确生成
- [ ] 包围算法正常工作（4个内圈+外圈）
- [ ] Boss房间特殊镜头演出正常
- [ ] Performance: 房间内10敌人时帧率 ≥ 60fps
