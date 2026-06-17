# ADR-008: 第三人称相机系统

- **Status**: Accepted
- **Date**: 2026-06-17
- **Engine**: Unreal Engine 5.7
- **Supersedes**: 2.5D 固定侧视相机

## Context

3D 第三人称动作游戏（崩坏3 式）需要复杂的相机系统：
- 自由视角探索
- 战斗锁定跟随
- 闪避子弹时间
- 必杀特写演出
- 避免穿墙/抖动

相机直接影响玩家操作感和战斗体验——相机穿墙会让玩家迷失，镜头抖动会让玩家晕眩，跟随不平滑会让战斗不流畅。

## Decision

**使用 UE5 的 `USpringArmComponent` + `UCameraComponent` 结构，配合自定义 `UCameraController` 组件管理 4 种相机模式。**

### 相机硬件结构

```
PlayerCharacter (ACharacter)
└── USpringArmComponent
    ├── TargetArmLength: 250cm (自由) / 200cm (锁定)
    ├── bDoCollisionTest: true
    ├── bEnableCameraLag: true (平滑跟随)
    ├── CameraLagSpeed: 10.0
    └── UCameraComponent
        ├── FOV: 65° (自由) / 55° (锁定)
        └── Post Process (模糊、色彩)
```

### 4 种相机模式

| 模式 | 触发条件 | 距离 | FOV | 行为 |
|---|---|---|---|---|
| **Free** | 非战斗状态 | 250cm | 65° | 玩家控制旋转，自动回正 |
| **Locked** | 锁定敌人 | 200cm | 55° | 目标居中，环绕跟随 |
| **Dodge** | 极限闪避 | 150cm | 50° | 子弹时间 + 拉近 |
| **Ultimate** | 必杀演出 | 100cm | 45° | 角色特写 + 背景虚化 |

### 相机模式状态机

```cpp
UCLASS()
class UCameraController : public UActorComponent
{
    GENERATED_BODY()

    UPROPERTY() ECameraMode CurrentMode = ECameraMode::Free;
    
    // 相机参数 (DataAsset 配置)
    UPROPERTY() FCameraModeParams FreeParams;
    UPROPERTY() FCameraModeParams LockedParams;
    UPROPERTY() FCameraModeParams DodgeParams;
    UPROPERTY() FCameraModeParams UltimateParams;
    
    // 切换方法
    UFUNCTION() void SetMode(ECameraMode NewMode);
    UFUNCTION() void UpdateCamera(float DeltaTime);
    
    // 内部状态
    FVector DesiredOffset;
    FRotator DesiredRotation;
    float TargetFOV;
};

UENUM()
enum class ECameraMode : uint8
{
    Free,       // 自由越肩
    Locked,     // 锁定跟随
    Dodge,      // 极限闪避
    Ultimate    // 必杀演出
};

USTRUCT()
struct FCameraModeParams
{
    GENERATED_BODY()
    UPROPERTY() float Distance = 250.f;
    UPROPERTY() float HeightOffset = 50.f;
    UPROPERTY() float FOV = 65.f;
    UPROPERTY() float LagSpeed = 10.f;
    UPROPERTY() float RotationSpeed = 3.f;
    UPROPERTY() bool bAutoCenter = false;
};
```

### 模式切换插值

所有模式切换都通过平滑插值，避免突兀跳变：

```cpp
void UCameraController::UpdateCamera(float DeltaTime)
{
    // 平滑过渡到目标参数
    CurrentDistance = FMath::FInterpTo(CurrentDistance, TargetDistance, DeltaTime, BlendSpeed);
    CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVBlendSpeed);
    
    // SpringArm 参数更新
    SpringArm->TargetArmLength = CurrentDistance;
    Camera->SetFieldOfView(CurrentFOV);
    
    // 锁定模式：目标在屏幕特定位置
    if (CurrentMode == ECameraMode::Locked)
    {
        FVector TargetScreenPos = ComputeLockedTargetScreenPos();
        // 调整相机角度让目标在屏幕上部 1/3 处
    }
}
```

### 碰撞处理

```
SpringArm 碰撞测试流程:
1. 从角色位置射线检测相机目标位置
2. 如果命中障碍物:
   a. SpringArm 自动拉近到碰撞点
   b. 尝试旋转避免遮挡 (左右微调)
3. 如果仍遮挡:
   a. 进入"近景模式"(距离 < 100cm)
   b. 角色半透明（避免遮挡视野）

特殊处理:
- 墙角：相机自动上移或侧移
- 低天花板：相机下压
- 多个障碍物：选择最小遮挡方向
```

### 自动回正 (Free 模式)

```cpp
// 玩家停止旋转相机 3 秒后，缓慢回到角色背后
void UCameraController::UpdateAutoCenter(float DeltaTime)
{
    TimeSinceLastInput += DeltaTime;
    
    if (TimeSinceLastInput > AutoCenterDelay)  // 3秒
    {
        FRotator TargetRotation = ComputeBehindPlayerRotation();
        CurrentYaw = FMath::FInterpTo(CurrentYaw, TargetRotation.Yaw, DeltaTime, AutoCenterSpeed);
    }
}
```

### 锁定模式跟随

```
锁定模式相机行为:
1. 目标始终位于屏幕中心偏上 1/3 位置
2. 目标移动时相机平滑跟随（Lag 0.1s）
3. 玩家环绕目标时相机同步旋转
4. 目标快速移动时镜头提前预判（leading）
5. 目标切换时相机平滑旋转（0.2s 过渡）
```

### 镜头演出

```cpp
// 必杀演出序列
void UCameraController::PlayUltimateSequence()
{
    SetMode(ECameraMode::Ultimate);
    
    // 阶段 1: 拉近至特写 (0.5s)
    PlayCameraAnimation(UltimateZoomIn, 0.5f);
    
    // 阶段 2: 特写持续 (1.0s) - 角色表情清晰可见
    // 应用景深效果
    ApplyDOF(0.3f);
    
    // 阶段 3: 必杀释放 (0.5s)
    // 镜头震动 + FOV 快速扩大
    PlayCameraShake(UltimateShake, 1.0f);
    
    // 阶段 4: 恢复战斗视角 (0.3s)
    SetMode(ECameraMode::Locked);
}
```

### 性能预算

| 操作 | 预算 |
|---|---|
| 相机位置更新 | < 0.1ms |
| 碰撞检测 | < 0.2ms |
| 插值计算 | < 0.05ms |
| 模式切换 | 无额外开销 |
| **总相机开销** | **< 0.4ms** |

### 关键参数（可调）

| 参数 | 默认值 | 安全范围 | 作用 |
|---|---|---|---|
| FreeDistance | 250cm | 200-400cm | 自由模式距离 |
| LockedDistance | 200cm | 150-300cm | 锁定模式距离 |
| FreeFOV | 65° | 55-75° | 自由模式视野 |
| LockedFOV | 55° | 45-65° | 锁定模式视野 |
| CameraLag | 10.0 | 5-20 | 跟随平滑度 |
| AutoCenterDelay | 3s | 2-5s | 自动回正延迟 |
| LockBlendTime | 0.2s | 0.1-0.5s | 锁定切换时间 |
| DodgeSlowMo | 0.3x | 0.2-0.5x | 极限闪避时间膨胀 |

## Consequences

### 优点

- ✅ **UE5 原生支持** — SpringArm + Camera 是 UE5 推荐方案
- ✅ **平滑跟随** — Lag/Interp 让镜头自然
- ✅ **碰撞避免** — SpringArm 内置碰撞测试
- ✅ **模式切换平滑** — 所有参数可插值过渡
- ✅ **数据驱动** — 相机参数在 DataAsset 中配置

### 缺点

- ⚠️ **穿墙复杂** — 复杂场景需要额外处理
- ⚠️ **锁定跟随难度** — 快速移动时可能抖动
- ⚠️ **性能成本** — 碰撞检测每帧一次
- ⚠️ **镜头演出复杂** — 需要 Sequencer 配合

### 风险缓解

- 使用 UE5 CameraBlocking 系统辅助穿墙处理
- 锁定跟随使用 Predictive Lag（预判目标位置）
- 相机碰撞检测用简化几何体
- 镜头演出用 Timeline + CameraAnim 组合

## Related Documents

- [ADR-005](adr-005-fixed-room-size.md): 3D 关卡空间
- [ADR-009](adr-009-lock-on.md): 锁定系统
- [Player Controller GDD](../../design/gdd/player-controller.md): Camera Modes 章节
