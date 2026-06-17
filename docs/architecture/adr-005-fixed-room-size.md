# ADR-005: 3D 竞技场式关卡空间

- **Status**: Accepted (Updated)
- **Date**: 2026-06-17 (updated from 2.5D fixed room)
- **Engine**: Unreal Engine 5.7
- **Supersedes**: ADR-005 v1.0 (Fixed Room Size for 2.5D)

## Context

格斗萌主转型为3D第三人称动作游戏（参考崩坏3）。原2.5D设计采用1920×1080固定尺寸的平面房间，摄像机固定在房间中心，角色在XZ平面（Y轴锁定）移动。这种设计适合2D横版视角，但无法支持3D空间战斗。

崩坏3的副本结构是"3D竞技场式房间"——每个房间是有限但立体的3D空间，玩家可以在X/Y/Z三个维度移动，利用空间走位闪避Boss攻击、绕后输出、跳上平台获得视角优势。这种设计既保持了副本闯关的"房间制"节奏感，又引入了3D空间的深度。

## Decision

**采用"3D竞技场式关卡空间"替代"固定尺寸房间"。**

### 核心规格

| 属性 | 值 | 说明 |
|---|---|---|
| **房间形状** | 立方体/不规则多边形 | 3D空间，不再是2D平面 |
| **大小范围** | 15m×15m×8m ~ 35m×35m×15m | 根据房间类型调整 |
| **边界方式** | 隐形墙 + 视觉边界 | 限制活动范围，保证战斗密度 |
| **垂直元素** | 平台、高低差、可破坏物件 | 增加空间维度（但非核心） |
| **摄像机** | 第三人称越肩 + 锁定跟随 | 不再是固定侧视 |

### 房间类型规格

| 类型 | 大小 | 敌人数 | 垂直元素 | 边界强度 |
|---|---|---|---|---|
| **Combat (小怪)** | 15m × 15m × 8m | 3-5 | 无或1个低平台 | 强（紧密空间） |
| **Elite (精英)** | 25m × 25m × 10m | 1-2 精英 | 2个平台 + 高低差 | 中（允许走位） |
| **Boss** | 35m × 35m × 15m | 1 Boss | 多层平台 + 可破坏柱子 | 弱（大空间） |
| **Treasure** | 10m × 10m × 6m | 0 | 宝箱台 + 装饰 | 强（小房间） |
| **Rest** | 12m × 12m × 6m | 0 | 恢复点 + 商店NPC | 强 |

### UE5 实现

#### 1. 房间 Actor

```cpp
UCLASS()
class ADungeonRoom : public AActor
{
    UPROPERTY() UBoxComponent* ArenaBounds;      // 竞技场边界
    UPROPERTY() UBoxComponent* TriggerVolume;    // 进入触发器
    UPROPERTY() TArray<ASpawnPoint*> SpawnPoints; // 敌人出生点
    UPROPERTY() TArray<APlatform*> Platforms;     // 平台列表
    
    UPROPERTY(EditAnywhere, Category="Arena")
    FVector ArenaSize = FVector(1500, 1500, 800); // 默认15m×15m×8m
    
    UPROPERTY(EditAnywhere, Category="Arena")
    ERoomType RoomType = ERoomType::Combat;
};
```

#### 2. 边界系统

```cpp
// 隐形墙边界 — 防止角色离开竞技场
ArenaBounds->SetCollisionProfileName(TEXT("ArenaBounds"));
ArenaBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
ArenaBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

// 视觉边界 — 给玩家明确的视觉提示（可选）
// 例如：发光的能量墙、石柱、悬崖边缘
```

#### 3. 房间流程

```
玩家进入 TriggerVolume 
    → 门关闭 + 边界激活
    → 敌人 Spawn 到 SpawnPoints
    → 玩家清完当前波次
    → 2秒安全窗口
    → 下一波 Spawn / 门开启 + 奖励
```

## Consequences

### 优点

- ✅ **3D空间深度** — 玩家可以利用空间走位、绕后、跳平台，战斗更有策略
- ✅ **镜头表现力** — 3D空间支持镜头拉近特写、旋转演出、必杀慢动作
- ✅ **Boss设计丰富** — Boss可以在3D空间移动、跳跃、破坏环境
- ✅ **视觉冲击** — Q版角色在3D空间释放糖果色爆炸，效果更震撼
- ✅ **保持节奏感** — 仍然是"房间制"闯关，不会变成开放世界
- ✅ **职业差异化** — 3D空间让远程/近战职业的差异更明显

### 缺点/挑战

- ⚠️ **镜头系统复杂** — 3D空间需要复杂的相机系统避免穿模
- ⚠️ **关卡设计工作量** — 每个房间是3D模型，不是简单的2D布局
- ⚠️ **AI寻路更复杂** — NavMesh 3D寻路比2D状态机复杂得多
- ⚠️ **战斗密度控制** — 空间太大会让战斗松散，太小会让镜头穿模
- ⚠️ **性能压力** — 3D空间需要更多碰撞检测和渲染

### 迁移影响

| 原 2.5D 代码 | 3D 迁移工作量 | 处理方式 |
|---|---|---|
| `DungeonRoom` 固定尺寸 | 中 | 重写为 3D ArenaBounds |
| `DungeonFlow` 房间流程 | 低 | 逻辑不变，适配3D |
| Camera 固定侧视 | 大 | 完全重写为第三人称 |
| PlayerCharacter 移动 | 中 | 解除 Y 轴锁定 |
| BaseEnemy AI | 中 | 2D状态机 → NavMesh |

## 参考

- **崩坏3**: 副本房间为3D竞技场，Boss战空间特别大
- **战双帕弥什**: 小副本房间紧凑，Boss战空间大
- **绝区零**: 3D副本空间 + 环境互动（可破坏物件）
- **DNF**: 虽然是2D但每个房间大小不同（可参考尺寸变化）

## Related ADRs

- [ADR-001](adr-001-cpp-primary.md): C++ 为主语言
- [ADR-007](adr-007-gas-combat.md): GAS 战斗框架
- [ADR-008](adr-008-camera-system.md): 第三人称相机系统 (TODO)
- [ADR-009](adr-009-lock-on.md): 锁定系统设计 (TODO)
