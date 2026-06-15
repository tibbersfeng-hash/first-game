# ADR-006: Frame-Based Combat Timing

## Status
Accepted

## Date
2026-06-15

## Engine Compatibility

| Field | Value |
|-------|-------|
| **Engine** | Godot 4.6.3 |
| **Domain** | Core / Physics |
| **Knowledge Risk** | HIGH — Godot 4.6 is post-LLM-cutoff |
| **References Consulted** | `docs/engine-reference/godot/VERSION.md`, `docs/engine-reference/godot/modules/physics.md`, `docs/engine-reference/godot/breaking-changes.md` |
| **Post-Cutoff APIs Used** | None — _physics_process and frame counters are stable since 4.0 |
| **Verification Required** | 确认 Godot 4.6 中 _physics_process 在 60fps 固定步长下稳定运行；验证 Engine.physics_ticks_per_second 设置行为 |

## ADR Dependencies

| Field | Value |
|-------|-------|
| **Depends On** | ADR-001 (GDScript — 战斗逻辑在 GDScript 中执行) |
| **Enables** | Combat System (帧数据定义); Combo System (取消窗口基于帧); Hit Detection (命中判定基于帧) |
| **Blocks** | All combat timing implementation |
| **Ordering Note** | 这是最核心的战斗决策——所有帧数据、取消窗口、输入缓冲都依赖此决策，必须在任何战斗代码前确定 |

## Context

### Problem Statement
格斗游戏的核心体验来自帧精确的时序控制——"这招启动 2 帧、判定 3 帧、收招 4 帧"。传统格斗游戏用帧数（frame count）定义一切时序，因为帧是游戏逻辑的最小时间单位。但 Godot 的 _physics_process 提供的是 delta 时间（秒），且物理帧率可能波动。战斗系统应该用帧数还是秒数定义时序？

### Constraints
- 参考游戏 DNF 和传统格斗游戏用帧数定义所有战斗时序
- 60fps 下 1 帧 = 16.67ms，是玩家可感知的最小时间单位
- 格斗游戏的"手感"高度依赖帧精确的取消窗口（差 1 帧就是"按早了"vs"按晚了"）
- Godot 的 _physics_process 默认 60 ticks/s，与 60fps 对齐

### Requirements
- 所有战斗时序定义精度到帧
- 取消窗口、输入缓冲、硬直时间必须帧精确（不因帧率波动而改变）
- 伤害计算、命中判定必须在逻辑帧内同步完成
- 需要支持帧步进调试（暂停 → 逐帧推进）

## Decision

战斗系统使用**帧数（frame count）**定义所有时序。Godot 的 `Engine.physics_ticks_per_second` 固定为 60，`_physics_process` 每帧递增帧计数器。所有战斗逻辑在 `_physics_process` 中执行，不使用 `_process`。

### 架构图

```
┌─────────────────────────────────────────────────────┐
│              Frame-Based Timing System               │
│                                                      │
│  Engine.physics_ticks_per_second = 60 (固定)         │
│                                                      │
│  _physics_process(delta):                            │
│    1. frame_counter += 1                              │
│    2. InputBuffer.process_frame()    ← 读取输入       │
│    3. StateMachine.physics_update()  ← 状态推进       │
│       │                                              │
│       ├─ Idle: 检查输入 → 可能转换到 Attack          │
│       ├─ Attack:                                      │
│       │   ├─ if frame_in_state < startup_frames:     │
│       │   │    → startup 阶段（不可取消）            │
│       │   ├─ elif frame_in_state < startup+active:   │
│       │   │    → active 阶段（判定命中）             │
│       │   └─ else:                                    │
│       │        → recovery 阶段（检查取消窗口）        │
│       └─ HitStun: frame_in_state < stun_frames       │
│                                                      │
│    4. HitDetection.evaluate()        ← 命中检测       │
│    5. ComboManager.check_cancel()    ← 取消判定       │
│    6. SignalBus.emit()               ← 事件通知       │
└─────────────────────────────────────────────────────┘
```

### 帧计数器实现

```gdscript
# 在每个有状态实体中
var frame_counter: int = 0
var frame_in_state: int = 0

func _physics_process(_delta: float) -> void:
    frame_counter += 1
    frame_in_state += 1
    # 状态逻辑基于 frame_in_state 判断

func transition_to(state_name: String) -> void:
    frame_in_state = 0  # 重置帧计数
    current_state = states[state_name.to_lower()]
    current_state.enter()
```

### 帧数据查询

```gdscript
# CombatData (Autoload) 提供帧数据
func get_attack_phase(attack_id: String, frame: int) -> String:
    var data = frame_data.get(attack_id, {})
    var startup = data.get("startup_frames", 0)
    var active = data.get("active_frames", 0)
    
    if frame < startup:
        return "startup"
    elif frame < startup + active:
        return "active"
    else:
        return "recovery"

# 取消窗口判定
func is_in_cancel_window(attack_id: String, frame: int) -> bool:
    var data = frame_data.get(attack_id, {})
    var startup = data.get("startup_frames", 0)
    var active = data.get("active_frames", 0)
    var recovery = data.get("recovery_frames", 0)
    var cancel_window_pct = data.get("cancel_window_pct", 0.5)
    
    var recovery_start = startup + active
    var recovery_end = recovery_start + recovery
    var cancel_end = recovery_start + int(recovery * cancel_window_pct)
    
    return frame >= recovery_start and frame < cancel_end
```

### 项目设置

```
# project.godot 关键设置
[physics]
common/physics_ticks_per_second=60
common/physics_interpolation=false  # 战斗逻辑不需要插值
2d/default_gravity=0                # 横版格斗无重力（跳跃是预定义弧线）
```

### 帧步进调试支持

```gdscript
# 调试工具：帧步进
var debug_step_mode: bool = false
var debug_step_frames: int = 0

func _unhandled_input(event: InputEvent) -> void:
    if debug_step_mode:
        if event.is_action_pressed("debug_step_forward"):
            debug_step_frames = 1  # 推进 1 帧
        elif event.is_action_pressed("debug_step_10"):
            debug_step_frames = 10  # 推进 10 帧

func _physics_process(_delta: float) -> void:
    if debug_step_mode and debug_step_frames <= 0:
        return  # 暂停逻辑更新
    if debug_step_mode:
        debug_step_frames -= 1
    # ... 正常帧逻辑
```

## Alternatives Considered

### Alternative 1: Delta-Time Based Timing
- **Description**: 所有时序用秒数定义（如 startup=0.033s），用 `_physics_process(delta)` 累积时间
- **Pros**: 帧率无关——30fps/60fps/120fps 行为一致；Godot 默认推荐方式
- **Cons**: 取消窗口精度不够——0.033s 的窗口在 30fps 下只有 1 帧，在 60fps 下有 2 帧，手感不一致；浮点累积误差导致帧数偏移
- **Rejection Reason**: 格斗游戏的核心体验依赖帧精确时序，delta-time 无法保证

### Alternative 2: Hybrid (Frame + Delta)
- **Description**: 战斗逻辑用帧数，非战斗（动画、特效）用 delta
- **Pros**: 战斗精确，视觉效果平滑
- **Cons**: 两套时间系统增加认知负担；战斗和视觉的时序可能不同步
- **Rejection Reason**: 增加复杂度，MVP 阶段统一帧数更简单

### Alternative 3: Fixed Timestep with Substeps
- **Description**: 自定义固定时间步长，支持子步模拟
- **Cons**: 过度工程化——格斗游戏不需要物理子步；增加实现复杂度
- **Rejection Reason**: MVP 不需要此复杂度

## Consequences

### Positive
- **帧精确** — 取消窗口、输入缓冲、硬直时间都是整数帧，手感一致可预测
- **DNF 一致** — 与参考游戏的时序模型完全一致
- **可调试** — 帧步进模式让开发者逐帧检查战斗状态
- **数据驱动** — 帧数天然是整数，适合 Resource 配置（ADR-004）

### Negative
- **帧率绑定** — 物理帧率必须固定 60fps，不支持动态调整
- **帧率波动** — 如果设备无法维持 60fps，游戏会慢动作而非丢帧（因 _physics_process 固定步长）
- **无帧率缩放** — 未来若支持 120fps 模式，需要重写帧数据或实现帧数缩放

### Risks
- **风险**: 低端设备无法维持 60fps → **缓解**: 最低配置要求 60fps（PC 平台，2D 游戏几乎无此问题）；可接受慢动作而非丢帧
- **风险**: 未来 120fps 支持需求 → **缓解**: 帧数据抽象为 `frame_count * (60 / target_fps)` 的缩放公式
- **风险**: _physics_process 和 _process 不同步 → **缓解**: 战斗逻辑 100% 在 _physics_process 中，视觉动画可延迟 1 帧无感

## GDD Requirements Addressed

| GDD System | Requirement | How This ADR Addresses It |
|------------|-------------|--------------------------|
| combat-system.md | 启动帧/判定帧/收招帧用帧数定义 | frame_in_state 与 AttackData.startup_frames / active_frames / recovery_frames 直接比较 |
| combat-system.md | 取消窗口 = recovery 帧的前 50% | `is_in_cancel_window()` 用帧数精确计算 |
| combo-system.md | 输入缓冲窗口 8 帧 | InputBuffer 用帧计数器追踪输入有效期 |
| input-system.md | 输入缓冲基于帧数 | InputBuffer.process_frame() 每帧调用 |
| hit-detection-feedback.md | 命中停顿 3-8 帧 | HitStopManager 用帧计数器控制停顿时长 |
| player-controller.md | 状态机转换基于帧精确时机 | StateMachine.physics_update() 基于 frame_in_state 判断 |

## Performance Implications
- **CPU**: 帧数比较是整数运算 — 极快，每帧 < 0.01ms
- **Memory**: 每个实体增加 2 个 int (frame_counter, frame_in_state) — 可忽略
- **Load Time**: 无额外开销
- **Network**: 不适用（MVP 无 PVP）

## Migration Plan
无 — 新项目初始决策。未来如需支持可变帧率：
1. 将所有帧数定义改为 `帧数 * frame_scale`，frame_scale = 60 / target_fps
2. 在 CombatData 中增加 frame_scale 配置
3. 所有帧比较改为 `frame_in_state < startup_frames * frame_scale`

## Validation Criteria
1. Engine.physics_ticks_per_second 设置为 60
2. 所有战斗逻辑在 _physics_process 中执行，不在 _process 中
3. 状态转换基于 frame_in_state 整数比较，不使用 delta 时间累积
4. 取消窗口判定帧精确（整数帧边界）
5. 帧步进调试模式可正常工作（暂停 → 逐帧推进）

## Related Decisions
- ADR-001: GDScript as Primary Language
- ADR-004: Resource-Based Data Configuration (AttackData 定义帧数值)
- ADR-005: Fixed Room Size (房间尺寸不影响帧时序，但 Camera 行为需一致)
