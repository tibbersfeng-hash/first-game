# ADR-009: 目标锁定系统

- **Status**: Accepted
- **Date**: 2026-06-17
- **Engine**: Unreal Engine 5.7
- **Dependencies**: ADR-008 (Camera System)

## Context

3D 动作游戏的锁定系统直接影响战斗手感和空间感知：
- 锁定让攻击始终面向目标（减少挥空）
- 锁定让闪避相对目标方向（更精确）
- 锁定切换让玩家管理多个敌人
- 错误的锁定设计会让玩家"卡视角"或"锁定错目标"

参考崩坏3、战双帕弥什、原神的锁定实现。

## Decision

**使用自定义 `ULockOnComponent` 组件，基于球体+锥形范围检测可锁定目标，支持手动切换和自动重锁定。**

### 锁定范围

```
锁定范围结构:
- 球形范围: 半径 2000cm (20m)
- 锥形 FOV: ±60°（相对相机前方）
- 高度差: ±1000cm (上下 10m)

     ╱‾‾‾‾‾‾‾╲
    ╱   FOV    ╲
   ╱   60°      ╲
  ╱______________╲
  ← Player        →
  球形范围 20m
```

### 组件架构

```cpp
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ULockOnComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULockOnComponent();
    
    virtual void TickComponent(float DeltaTime, ...) override;
    
    // 锁定控制
    UFUNCTION(BlueprintCallable) void LockOnNearest();
    UFUNCTION(BlueprintCallable) void SwitchTarget(bool bNext);
    UFUNCTION(BlueprintCallable) void ReleaseLock();
    
    // 查询
    UFUNCTION(BlueprintPure) AActor* GetCurrentTarget() const;
    UFUNCTION(BlueprintPure) bool IsLockedOn() const;
    UFUNCTION(BlueprintPure) TArray<AActor*> GetLockableTargets() const;
    
    // 委托
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockChanged, AActor*, NewTarget);
    UPROPERTY(BlueprintAssignable) FOnLockChanged OnLockChanged;

protected:
    // 配置（DataAsset 驱动）
    UPROPERTY(EditAnywhere, Category="LockOn") float LockRange = 2000.f;
    UPROPERTY(EditAnywhere, Category="LockOn") float LockFOV = 60.f;
    UPROPERTY(EditAnywhere, Category="LockOn") float LockHeightRange = 1000.f;
    UPROPERTY(EditAnywhere, Category="LockOn") float SwitchDelay = 0.2f;
    UPROPERTY(EditAnywhere, Category="LockOn") float LostDelay = 3.f;
    
    // 目标优先级权重
    UPROPERTY(EditAnywhere) float WeightHP = 0.4f;       // HP 越低越优先
    UPROPERTY(EditAnywhere) float WeightDistance = 0.4f;  // 距离越近越优先
    UPROPERTY(EditAnywhere) float WeightAngle = 0.2f;     // 角度越小越优先
    
    // 状态
    UPROPERTY() TWeakObjectPtr<AActor> CurrentTarget;
    UPROPERTY() TArray<TWeakObjectPtr<AActor>> CachedTargets;
    UPROPERTY() float LastSwitchTime = 0.f;
    UPROPERTY() float TimeOutOfFOV = 0.f;

private:
    void UpdateLockableTargets();
    float CalculateTargetPriority(AActor* Target) const;
    bool IsTargetValid(AActor* Target) const;
    void HandleTargetLost();
};
```

### 目标优先级算法

```cpp
float ULockOnComponent::CalculateTargetPriority(AActor* Target) const
{
    // 1. 距离分数 (0-1, 越近越高)
    float Distance = FVector::Dist(PlayerLoc, Target->GetActorLocation());
    float DistanceScore = 1.f - FMath::Clamp(Distance / LockRange, 0.f, 1.f);
    
    // 2. 角度分数 (0-1, 越接近相机中心越高)
    FVector ToTarget = (Target->GetActorLocation() - PlayerLoc).GetSafeNormal();
    FVector CameraForward = GetCameraForward();
    float Angle = FMath::Acos(FVector::DotProduct(ToTarget, CameraForward));
    float AngleScore = 1.f - FMath::Clamp(Angle / FMath::DegreesToRadians(LockFOV), 0.f, 1.f);
    
    // 3. HP 分数 (0-1, HP 越低越高)
    float HPRatio = GetTargetHPRatio(Target);
    float HPScore = 1.f - HPRatio;
    
    // 加权综合
    return DistanceScore * WeightDistance 
         + AngleScore * WeightAngle 
         + HPScore * WeightHP;
}
```

### 目标筛选规则

| 规则 | 条件 | 说明 |
|---|---|---|
| 距离 | Distance ≤ LockRange (20m) | 超出范围不锁 |
| 角度 | 在 FOV 锥形内 (±60°) | 背后的不锁 |
| 高度 | 高度差 ≤ LockHeightRange | 太高太低不锁 |
| 存活 | HP > 0 | 死亡的不锁 |
| 可锁定 | `ILockableTarget` 接口 | 特定敌人可锁定 |
| 非队友 | 阵营 != 玩家阵营 | 友方不锁 |

### 目标丢失处理

```
目标丢失流程:
1. 目标离开 FOV → 开始计时 (TimeOutOfFOV)
2. 3秒内回到 FOV → 继续锁定
3. 3秒后仍离开 → 自动切换到次优先目标
4. 无其他目标 → 释放锁定

目标死亡流程:
1. 当前目标 HP ≤ 0
2. 立即切换到次优先目标 (无延迟)
3. 无其他目标 → 释放锁定

强制切换 (Tab 键):
1. 按下切换键
2. 延迟 SwitchDelay (0.2s) 防止抖动
3. 切换到下一优先级目标（按优先级排序循环）
```

### 锁定 UI

```
锁定标记:
- 被锁目标头顶显示特殊标记
- 标记颜色: 红(高HP) → 黄(中HP) → 绿(低HP)
- 标记随目标移动平滑跟随
- 切换时标记有过渡动画

候选目标提示:
- 范围内可锁定目标显示空心标记
- 锁定后实心化
- 失去目标时空心闪烁后消失

屏幕中心:
- 无锁定时显示准心
- 锁定后准心变为目标指示
```

### 与其他系统集成

```
Camera System:
    ← OnLockChanged(NewTarget)
    → SetMode(Locked/Free)
    → 相机跟随目标

Player Controller:
    ← GetCurrentTarget()
    → 面向目标方向移动
    → 闪避相对目标方向

Combat System:
    ← GetCurrentTarget()
    → 攻击朝目标方向
    → HitBox 朝向调整

Enemy AI:
    ← IsLockedOn(EnemyActor)
    → AI 知道被锁定，可改变行为
```

### 性能预算

| 操作 | 预算 |
|---|---|
| 范围检测（每帧） | < 0.1ms |
| 优先级排序（切换时） | < 0.05ms |
| UI 标记更新 | < 0.05ms |
| **总锁定开销** | **< 0.2ms** |

### 关键参数（可调）

| 参数 | 默认值 | 安全范围 | 作用 |
|---|---|---|---|
| LockRange | 2000cm | 1500-3000cm | 锁定范围 |
| LockFOV | 60° | 45-90° | 锁定视角 |
| LockHeightRange | 1000cm | 500-2000cm | 高度差限制 |
| SwitchDelay | 0.2s | 0.1-0.5s | 切换防抖 |
| LostDelay | 3s | 2-5s | 丢失延迟 |
| WeightHP | 0.4 | 0-1 | HP 权重 |
| WeightDistance | 0.4 | 0-1 | 距离权重 |
| WeightAngle | 0.2 | 0-1 | 角度权重 |

## Consequences

### 优点

- ✅ **组件化** — 独立组件，易测试
- ✅ **数据驱动** — 所有参数可配置
- ✅ **优先级算法** — 自动选择最佳目标
- ✅ **防抖机制** — 避免快速切换抖动
- ✅ **平滑过渡** — 目标切换无突兀感

### 缺点

- ⚠️ **每帧检测** — 需要优化范围检测性能
- ⚠️ **多目标复杂** — 大量敌人时优先级计算复杂
- ⚠️ **视角遮挡** — 锁定目标可能被地形遮挡

### 风险缓解

- 缓存可锁定目标列表，每 0.5s 更新一次（非每帧）
- 使用 SphereOverlap + 手动角度过滤（比 LineTrace 快）
- 目标被遮挡时显示轮廓（PostProcess Stencil）
- 大量敌人时降低检测频率

## Related Documents

- [ADR-008](adr-008-camera-system.md): 相机系统（锁定模式）
- [Player Controller GDD](../../design/gdd/player-controller.md): Lock-On 章节
- [Combat System GDD](../../design/gdd/combat-system.md): 锁定影响
