# Input System (3D 版)

> **Status**: Draft (Updated for 3D)
> **Last Updated**: 2026-06-17
> **Implements Pillar**: 爽快打击+Q版反差 (输入响应是打击感的基石)
> **Reference**: 崩坏3、战双帕弥什、UE5 Enhanced Input

## Summary

输入系统管理键盘/手柄/鼠标的所有玩家输入映射、输入缓冲、优先级裁决、双模态视角控制（移动+视角分离）和上下文切换。它是战斗手感的基石——延迟1帧就是手感的生死之差。本系统为所有上层系统提供统一的输入事件接口，基于 UE5 Enhanced Input System。

> **Quick reference** — Layer: `Foundation` · Priority: `MVP` · Key deps: `None`

## Overview

格斗萌主的输入系统负责将物理输入转换为游戏动作事件。核心特性：
- **移动与视角分离**: 左摇杆/WASD控制移动，右摇杆/鼠标控制视角（3D必需）
- **输入缓冲**: 攻击/跳跃/闪避有缓冲窗口，让连招更宽容
- **上下文切换**: 同一输入在不同状态下映射不同动作
- **优先级裁决**: 多输入冲突时按优先级处理
- **双模态**: 自由模式（探索）和战斗模式（锁定）的输入差异

## Detailed Design

### Core Rules

1. **双轴分离**: 移动轴(XY)和视角轴(XY)独立处理
2. **输入缓冲**: 动作有 buffer_window，窗口内输入在当前动作结束后自动触发
3. **上下文切换**: 同一按键在自由模式/战斗模式/空中/地面有不同行为
4. **优先级裁决**: 闪避 > 必杀 > 攻击 > 锁定切换 > 移动
5. **输入消抖**: 同一输入 action_pressed 后 100ms 内不重复触发
6. **手柄/键鼠对等**: 所有操作两种方式完全对等，无强制

### Input Map Definition

#### 移动 (Move — 左摇杆 / WASD)

| Action | Keyboard | Gamepad | Context |
|---|---|---|---|
| Move_Forward | W | 左摇杆↑ | 全局（相对相机/锁定目标） |
| Move_Backward | S | 左摇杆↓ | 全局 |
| Move_Left | A | 左摇杆← | 全局 |
| Move_Right | D | 左摇杆→ | 全局 |
| Sprint | Shift | LB (按住) | 全局（需体力） |

#### 战斗 (Combat — 面部按钮)

| Action | Keyboard | Gamepad | Context | Notes |
|---|---|---|---|---|
| Attack_Light | J (鼠标左键) | X | 战斗 | 轻攻击，快速出招 |
| Attack_Heavy | K (鼠标右键) | Y | 战斗 | 重攻击，蓄力可选 |
| Skill_1 | U | A (底部) | 战斗 | 技能1（短CD） |
| Skill_2 | I | B (右侧) | 战斗 | 技能2（中CD） |
| Skill_Ult | O (需能量) | RT (需能量) | 战斗 | 必杀技（能量≥30） |

#### 防御/闪避 (Defense — 肩键)

| Action | Keyboard | Gamepad | Context | Notes |
|---|---|---|---|---|
| Dodge | Space | RB | 战斗/自由 | 3D全向闪避，无敌帧 |
| Lock_On | Tab | R3 (按下右摇杆) | 战斗 | 锁定/切换目标 |

#### 视角 (Camera — 右摇杆 / 鼠标)

| Action | Keyboard | Gamepad | Context | Notes |
|---|---|---|---|---|
| Camera_Pitch | 鼠标Y轴 | 右摇杆Y | 全局 | 上下视角 |
| Camera_Yaw | 鼠标X轴 | 右摇杆X | 全局 | 左右视角 |
| Camera_Reset | 中键 | R3快速双击 | 全局 | 回到默认视角 |
| Lock_Release | Esc | L3 (按下左摇杆) | 战斗 | 释放锁定 |

#### 菜单/其他 (Menu)

| Action | Keyboard | Gamepad | Context |
|---|---|---|---|
| Interact | E | Y (长按) | 非战斗 |
| Pause | Esc | Start | 全局 |
| QuickMenu | Q | Back | 全局 |
| SwitchTarget_Next | Tab (连按) | R (连按) | 战斗 |
| SwitchTarget_Prev | Shift+Tab | L (连按) | 战斗 |

### Input Buffer Windows

| Action | Buffer Window | Rationale |
|---|---|---|
| Attack_Light | 200ms (12帧) | 连招宽容度 |
| Attack_Heavy | 150ms (9帧) | 略短（重攻击更慎重） |
| Skill_1/2 | 200ms | 技能衔接 |
| Dodge | 100ms (6帧) | 闪避要及时 |
| Jump | 150ms | 配合土狼时间 |
| Lock_On | 50ms | 锁定要精确 |
| Ultimate | 300ms | 必杀时机要宽容 |

### Priority Rules (多输入冲突)

当多个输入在同一帧内到达，按以下优先级处理：

```
1. Dodge (最高) — 生存必需，必须即时响应
2. Ultimate — 高价值输入，避免被覆盖
3. Attack_Light / Attack_Heavy / Skill_* — 战斗动作
4. Lock_On — 目标管理
5. Jump — 移动动作
6. Sprint — 移动动作
7. Move_* (最低) — 持续输入，不冲突
```

**冲突处理规则**:
- 高优先级输入**覆盖**低优先级输入（不被缓冲）
- 同优先级输入按"最后输入"原则处理
- 闪避可**中断**攻击收招（但不可中断攻击启动）
- 攻击动作期间，移动输入被**忽略**（除非是连招推力）

### Context State Machine

```
           ┌──────────────────────┐
           │   Global Context     │
           │  (Move+Camera 可用)  │
           └──────┬───────────┬───┘
                  │           │
           ┌──────▼─────┐ ┌──▼──────────┐
           │ Free Mode  │ │ Combat Mode │
           │ (无锁定)   │ │ (锁定中)    │
           └──────┬─────┘ └──┬──────────┘
                  │           │
           ┌──────▼─────┐ ┌──▼──────────┐
           │ Ground     │ │ Air         │
           │ (地面)     │ │ (空中)      │
           └────────────┘ └─────────────┘
```

| Context | Attack_Light | Attack_Heavy | Dodge | Lock_On | Move |
|---|---|---|---|---|---|
| Free+Ground | 地面轻攻 | 地面重攻 | 地面闪避 | 锁定最近 | 相对相机 |
| Free+Air | 空中轻攻(1次) | 空中重攻(1次) | 不允许 | 锁定最近 | 空中控制 |
| Combat+Ground | 面向目标轻攻 | 面向目标重攻 | 相对目标闪避 | 切换目标 | 环绕目标 |
| Combat+Air | 面向目标空攻 | 不允许 | 不允许 | 切换目标 | 空中控制 |

### Dual Input Support (键鼠 / 手柄自动切换)

```cpp
// 自动检测最后使用的输入设备
if (LastInputDeviceChanged()) {
    SwitchInputMappingContext(LastInputDevice);
    // 更新 UI 提示（"键盘模式" / "手柄模式"）
}
```

| Device | Movement | Camera | Attacks | Dodge |
|---|---|---|---|---|
| Keyboard+Mouse | WASD | 鼠标移动 | J/K/U/I/O (数字键) | Space |
| Gamepad | 左摇杆 | 右摇杆 | X/Y/A/B (面部键) | RB |

## Interactions with Other Systems

| System | Interaction | Interface |
|---|---|---|
| Player Controller | 提供输入事件 | GetMovementInput(), IsActionPressed() |
| Combat System | 战斗输入消费 | OnAttackInput(), OnSkillInput() |
| Camera System | 视角输入 | GetCameraInput() → FVector2D |
| LockOn System | 锁定输入 | OnLockOnPressed(), OnSwitchTarget() |
| UI System | 菜单输入 | OnMenuInput(), OnPause() |
| Input Buffer | 提供缓冲窗口 | SetBufferWindow(Action, Duration) |

## Formulas

### 移动输入归一化（防止斜向超速）

```cpp
FVector2D MoveInput = GetMovementInput();
if (MoveInput.Size() > 1.0f) {
    MoveInput.Normalize();
}
```

### 相机输入平滑

```cpp
// 键鼠直接输入，手柄带死区+曲线
float CameraYaw = ApplyDeadzone(RawInput.X, 0.15f);
CameraYaw = FMath::Sign(CameraYaw) * FMath::Pow(FMath::Abs(CameraYaw), 1.4f);
// 非线性曲线，中心更精确，边缘更快
```

### 输入缓冲超时

```cpp
if (ActionBuffered && (CurrentTime - BufferTime) < BufferWindow) {
    ExecuteAction(BufferedAction);
    ClearBuffer(BufferedAction);
} else {
    ClearBuffer(BufferedAction);  // 超时丢弃
}
```

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|---|---|---|
| 同时按攻击+闪避 | 闪避优先 | 生存必需 |
| 攻击中按反方向移动 | 忽略（攻击推力除外） | 保持攻击朝向 |
| 锁定中快速切Tab | 200ms延迟后切换 | 防止抖动 |
| 手柄+键盘同时输入 | 最后使用的设备优先 | 避免双设备冲突 |
| 技能CD中按技能键 | 按键无效+UI提示 | 明确反馈 |
| 能量不足按必杀 | 按键无效+UI提示 | 明确反馈 |
| 空中按闪避 | 按键无效 | 防止无限浮空 |
| 菜单打开时按攻击 | 忽略 | 避免误操作 |

## Tuning Knobs

| Parameter | Default | Safe Range | Effect |
|---|---|---|---|
| light_attack_buffer | 200ms | 100-400ms | 连招宽容度 |
| dodge_buffer | 100ms | 50-200ms | 闪避响应性 |
| input_debounce | 100ms | 50-200ms | 防止重复触发 |
| camera_sensitivity | 1.0 | 0.5-2.0 | 视角旋转速度 |
| camera_invert_y | false | bool | 视角Y轴反转 |
| deadzone_inner | 0.15 | 0.05-0.3 | 摇杆死区 |
| deadzone_outer | 0.9 | 0.7-1.0 | 摇杆最大值 |
| device_switch_delay | 500ms | 200-1000ms | 设备切换延迟 |

## Acceptance Criteria

- [ ] 所有输入设备自动检测并切换映射
- [ ] 输入缓冲在窗口内100%成功触发
- [ ] 优先级规则：闪避 > 攻击 > 移动（通过测试验证）
- [ ] 键鼠和手柄所有操作100%对等
- [ ] 输入响应延迟 < 16ms（1帧内）
- [ ] 上下文切换正确（Free/Combat × Ground/Air）
- [ ] 无输入丢失（快速连续按键100%响应）
- [ ] Performance: 输入处理耗时 < 0.1ms per frame
