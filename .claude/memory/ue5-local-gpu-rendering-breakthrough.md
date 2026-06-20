---
name: ue5-local-gpu-rendering-breakthrough
description: UE5.7 本地 GPU 渲染环境变量、编辑器启动方案、sleep 策略
metadata: 
  node_type: memory
  type: reference
  originSessionId: 43872f51-7434-4cad-a99a-099c5e3aeda2
---

## UE5 编辑器本地 GPU 渲染方案

**核心发现**: 设置 `VK_ICD_FILENAMES` 环境变量后，UE5 编辑器可直接使用本地 RTX 3090 进行 Vulkan 渲染。

### 环境变量

```bash
export DISPLAY=:99                                    # Xvfb + x11vnc（端口 5902）
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/nvidia_icd.json  # 指定 NVIDIA Vulkan ICD
```

**必须同时设置两个变量**，缺一不可。

### 编辑器启动命令（已验证）

```bash
# 编辑器模式（GUI + Python）
nohup /home/vipuser/ue5/Engine/Binaries/Linux/UnrealEditor \
  /home/vipuser/first-game/src_ue5/FirstGame.uproject \
  -stdout -FullStdOutLogOutput -nosplash \
  -WINDOWPOSX=0 -WINDOWPOSY=0 -ResX=1280 -ResY=720 \
  > /tmp/ue5_editor.log 2>&1 &

# Headless 资产导入（commandlet 模式）
/home/vipuser/ue5/Engine/Binaries/Linux/UnrealEditor-Cmd \
  /home/vipuser/first-game/src_ue5/FirstGame.uproject \
  -stdout -FullStdOutLogOutput -unattended -nosplash -nullrhi
```

###  Sleep 策略（本机性能很好）

**本机配置**: Intel Xeon Gold 6248R @ 3.00GHz + RTX 3090 + 47GB RAM

**策略**: 使用 **10 秒 sleep + 多次检查**，不用长 sleep（120s/180s）。

```bash
# ✅ 推荐模式
export FIRSTGAME_SOME_TASK=1
nohup ... > /tmp/ue5_task.log 2>&1 &
PID=$!

for i in $(seq 1 30); do     # 最多等 300 秒
  sleep 10                    # 10 秒一次检查
  if grep -q "COMPLETE_MARKER" /tmp/ue5_task.log 2>/dev/null; then
    echo "✅ Done at ${i}x10s"
    break
  fi
  if ! kill -0 $PID 2>/dev/null; then
    echo "Process ended at ${i}x10s"
    break
  fi
done
```

**不要**:
- ❌ `sleep 120` 或更长的一次性等待
- ❌ `timeout 300` 然后只看最后结果

**要**:
- ✅ `sleep 10` + 循环检查 log 关键字
- ✅ 检查 PID 是否存活
- ✅ 最多 30 次循环（300 秒上限）

### Python 脚本执行注意

- **编辑器模式**: 通过 `init_unreal.py` + 环境变量钩子触发脚本
- **Headless 模式**: `-RunPythonScript` 在 `-nullrhi -unattended` 下**不执行**
- `unreal.log()` 输出在 `-unattended` 模式下**不会出现在 stdout**

### Python API 类型限制

- `Constant3Vector.constant` → 需要 `unreal.LinearColor`，不是 `unreal.Vector`
- `DirectionalLightComponent.LightColor` → 需要 `unreal.Color`（0-255），不是 `LinearColor`（0-1）
- `connect_material_expressions()` 的 `to_expression` 必须是 `MaterialExpression`，不能是 `Material`
- `MaterialInstanceConstant.set_vector_parameter_value` **不存在**
- `SkeletalMesh.set_editor_property('materials', [mi])` **类型不匹配**

### 已知限制

1. **材质参数无法通过 Python 设置** — 必须在 GUI 编辑器中手动操作
2. **LOD source 无法通过 Python 导入** — 需要 GUI 编辑器
3. **AnimBP 状态机无法通过 Python 编辑** — 需要 GUI 编辑器

### AnimBP 状态机替代方案（首选）

**问题**: UE5 Python API 无法编辑 AnimGraph（Blueprint 状态机）

**首选方案**: 创建 C++ `UAnimInstance` 子类

```cpp
// 头文件
UCLASS()
class UMonsterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) float Speed;
    UPROPERTY(BlueprintReadWrite) uint32 bIsAttacking : 1;
    UPROPERTY(BlueprintReadWrite) uint32 bIsHit : 1;
    UPROPERTY(BlueprintReadWrite) uint32 bIsDead : 1;

    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};

// 实现
void UMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // 自动获取速度
    if (APawn* Pawn = TryGetPawnOwner())
    {
        if (UCharacterMovementComponent* MoveComp = 
            Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
        {
            Speed = MoveComp->Velocity.Size2D();
        }
    }

    // 状态机逻辑
    if (CurrentState == TEXT("Idle")) { ... }
    else if (CurrentState == TEXT("Locomotion")) { ... }
    // ...
}
```

**优点**:
- ✅ 完全绕过 Blueprint AnimGraph 编辑
- ✅ 可测试、可调试、可版本控制
- ✅ 性能优于 Blueprint
- ✅ 逻辑清晰，易于维护

**已实现**: `Source/FirstGame/Public/Anim/MonsterAnimInstance.h/cpp`

### 环境信息

| 项 | 值 |
|---|---|
| UE5 版本 | 5.7.4 (CL-51494982) |
| 安装路径 | `/home/vipuser/ue5/` |
| 项目路径 | `/home/vipuser/first-game/src_ue5/` |
| GPU | NVIDIA GeForce RTX 3090 (24GB) |
| 显示服务 | Xvfb :99 + x11vnc (端口 5902) |
| VNC | TigerVNC :1 (端口 5901, localhost only) |
| Vulkan ICD | `/usr/share/vulkan/icd.d/nvidia_icd.json` |
| Python 脚本目录 | `src_ue5/Content/Python/` |
| 截图输出 | `src_ue5/Saved/Screenshots/LinuxEditor/` |

### 启动前清理

```bash
pkill -9 -f UnrealEditor 2>/dev/null
sleep 3  # 等待共享内存释放
```

### Sleep 策略（本机性能很好）

**本机配置**: Intel Xeon Gold 6248R @ 3.00GHz + RTX 3090 + 47GB RAM

**策略**: 使用 **10 秒 sleep + 多次检查**，不用长 sleep（120s/180s）。

```bash
# ✅ 推荐模式
export FIRSTGAME_SOME_TASK=1
nohup ... > /tmp/ue5_task.log 2>&1 &
PID=$!

for i in $(seq 1 30); do     # 最多等 300 秒
  sleep 10                    # 10 秒一次检查
  if grep -q "COMPLETE_MARKER" /tmp/ue5_task.log 2>/dev/null; then
    echo "✅ Done at ${i}x10s"
    break
  fi
  if ! kill -0 $PID 2>/dev/null; then
    echo "Process ended at ${i}x10s"
    break
  fi
done
```

**不要**: `sleep 120` 或更长的一次性等待  
**要**: `sleep 10` + 循环检查 log 关键字 + 检查 PID

### 糖果地牢 4 怪物管线状态 (2026-06-21)

**已完成**:
- ✅ 混元3D AI 生成 4 怪物 + 6 绑骨动作/怪物
- ✅ Blender 批量后处理 (拓扑/LOD/导出)
- ✅ UE5 headless 导入 52 uassets
- ✅ C++ `ConfigureMonsterAssets()` 自动加载 mesh/AnimBP + 动态彩色材质
- ✅ C++ `MonsterAnimInstance` 状态机（替代 Blueprint AnimGraph）
- ✅ UE5 Editor GUI 可用 (Xvfb + Vulkan)
- ✅ 4 ABaseEnemy 实例在编辑器中生成并截图验证
- ✅ Git 分支 `monster-3d-pipeline` (11 commits, pushed)

**剩余 (需 GUI 编辑器操作)**:
- ~~AnimBP 状态机配置~~ ✅ 已用 C++ 解决
- NPR 完整参数 (2-tone + Rim + Outline)
- LOD 关联
- 战斗整合测试
- P0 评审
