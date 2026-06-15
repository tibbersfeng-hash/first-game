# ADR-005: Fixed Room Size (1280×720, No Scrolling)

## Status
Proposed

## Date
2026-06-15

## Engine Compatibility

| Field | Value |
|-------|-------|
| **Engine** | Godot 4.6.3 |
| **Domain** | Rendering / Core |
| **Knowledge Risk** | HIGH — Godot 4.6 is post-LLM-cutoff |
| **References Consulted** | `docs/engine-reference/godot/VERSION.md`, `docs/engine-reference/godot/modules/rendering.md` |
| **Post-Cutoff APIs Used** | None — Camera2D and viewport settings are stable |
| **Verification Required** | 确认 1280×720 在目标 PC 分辨率下 letterboxing 行为；验证 Camera2D limit 属性正确约束视野 |

## ADR Dependencies

| Field | Value |
|-------|-------|
| **Depends On** | None |
| **Enables** | Dungeon Room System (房间布局基于固定尺寸); HUD System (UI 锚定基于固定分辨率) |
| **Blocks** | Room scene creation; Camera2D configuration; HUD layout |
| **Ordering Note** | 必须在房间场景和 HUD 布局制作前决定，影响所有空间相关的视觉设计 |

## Context

### Problem Statement
格斗萌主的每个副本房间需要确定可视范围和摄像机行为。DNF 式横版格斗的战斗区域是固定一屏，但 Godot 的默认 Camera2D 支持平滑跟随和滚动。是否使用固定房间+无滚动，还是允许摄像机跟随玩家？

### Constraints
- 参考原型 DNF 的战斗区域是固定一屏，玩家在屏幕内移动
- HUD 布局需要固定的锚定位置（左上 HP 条、右下技能栏）
- 所有敌人和玩家必须同时可见，不允许屏幕外敌人偷袭
- MVP 阶段需最小化实现复杂度

### Requirements
- 所有战斗内容在单屏内可见
- HUD 元素位置固定，不随角色移动
- 房间边界明确，玩家和敌人不可移出可视区域
- 简化实现，减少摄像机相关 bug

## Decision

每个战斗房间**固定为 1280×720 像素**，摄像机不跟随、不滚动。Camera2D 设置为固定位置居中，使用 limit 属性锁定视野范围。

### 架构图

```
┌──────────────────────────────────────────────────┐
│                1280 × 720 (Fixed)                │
│                                                  │
│   ┌────────────────────────────────────────┐     │
│   │         Camera2D (Fixed)               │     │
│   │  position = (640, 360)                 │     │
│   │  limit_left = 0, limit_right = 1280    │     │
│   │  limit_top = 0, limit_bottom = 720     │     │
│   │  smoothing_enabled = false             │     │
│   └────────────────────────────────────────┘     │
│                                                  │
│   Player ←→ Enemies (all visible)               │
│   Floor at y ≈ 560                               │
│   Walls at x = 0 and x = 1280                    │
│                                                  │
│   [HP] [Energy]                     Room 3/8     │  ← HUD (CanvasLayer, 不受 Camera 影响)
│                                          [Skills]│
└──────────────────────────────────────────────────┘
```

### Camera2D 配置

```gdscript
# 房间场景中的摄像机配置
camera = Camera2D.new()
camera.position = Vector2(640, 360)  # 房间中心
camera.limit_left = 0
camera.limit_right = 1280
camera.limit_top = 0
camera.limit_bottom = 720
camera.zoom = Vector2.ONE
camera.smoothing_enabled = false
camera.position_smoothing_enabled = false
```

### 房间边界实现

```gdscript
# StaticBody2D 墙壁
left_wall = StaticBody2D.new()
left_wall.position = Vector2(0, 360)
# CollisionShape2D: 阻止玩家和敌人移出 x < 0

right_wall = StaticBody2D.new()
right_wall.position = Vector2(1280, 360)
# CollisionShape2D: 阻止玩家和敌人移出 x > 1280
```

### HUD 独立层

```gdscript
# HUD 使用 CanvasLayer，不受 Camera2D 影响
hud = CanvasLayer.new()
hud.layer = 10  # 在游戏世界之上
# 所有 HUD 元素锚定到屏幕边缘
```

## Alternatives Considered

### Alternative 1: Smooth Camera Follow
- **Description**: Camera2D 平滑跟随玩家，房间可大于一屏
- **Pros**: 更大的战斗空间；Boss 战可利用全屏幕移动增加战术深度；更动态的视觉体验
- **Cons**: 需处理摄像机边界、死区、平滑参数调优；HUD 需 CanvasLayer 隔离；敌人可能在屏幕外
- **Rejection Reason**: MVP 阶段增加不必要的复杂度；DNF 式格斗体验不需要滚动

### Alternative 2: Multi-Screen Rooms (Scrolling Between Screens)
- **Description**: 大型房间由多个 1280×720 屏幕组成，玩家走到边缘时滚动到下一屏
- **Pros**: 更大的关卡空间；可做追逐战或探索型房间
- **Cons**: 需要屏幕切换逻辑；敌人跨屏管理复杂；滚动瞬间可能出现渲染异常
- **Rejection Reason**: 过度设计，MVP 不需要多屏房间

### Alternative 3: Dynamic Zoom
- **Description**: 根据战斗情况动态调整 Camera2D zoom（敌人多时缩小，1v1 时放大）
- **Cons**: 影响像素完美渲染；HUD 需要反向缩放保持尺寸；玩家对手感不稳定的抱怨
- **Rejection Reason**: 格斗游戏需要稳定的视觉参照，动态 zoom 会破坏距离感

## Consequences

### Positive
- **实现简单** — 无摄像机逻辑，无边界检查，无平滑调参
- **视觉稳定** — 玩家对距离和位置的判断始终精确
- **HUD 简化** — CanvasLayer 锚定即可，无需考虑 Camera 位移补偿
- **DNF 一致性** — 与参考游戏的视觉体验一致

### Negative
- **空间受限** — 1280×720 意味着横向约 1280 像素的活动空间，大型 Boss 可能显得拥挤
- **未来扩展受限** — 如果需要更大的战斗区域（如史诗 Boss 战），需要重构为滚动模式
- **分辨率适配** — 高分辨率显示器上需要 letterboxing 或拉伸

### Risks
- **风险**: Boss 战空间不够 → **缓解**: Boss 房可设计为玩家移动范围小、Boss 体型大的模式（DNF 的做法）
- **风险**: 非标分辨率显示问题 → **缓解**: Godot 的 stretch_mode 设置为 `canvas_items` + `keep_aspect`，保持 16:9 比例
- **风险**: 未来需要滚动模式时重构成本高 → **缓解**: 保留 Camera2D 节点但禁用功能，未来只需启用 smoothing + 调整 limit

## GDD Requirements Addressed

| GDD System | Requirement | How This ADR Addresses It |
|------------|-------------|--------------------------|
| dungeon-room.md | 每个房间固定尺寸，所有内容一屏可见 | 房间 1280×720，Camera2D 固定不动 |
| hud.md | HUD 布局在 1280×720 下设计，元素不遮挡战斗区 | CanvasLayer 隔离 HUD，锚定不受 Camera 影响 |
| player-controller.md | 玩家不可移出房间边界 | StaticBody2D 墙壁物理阻止越界 |
| enemy-ai.md | 追击范围不超过房间尺寸 | chase_range 设计在 1280px 以内 |
| combat-system.md | 所有战斗在一屏内发生 | 固定视野确保无屏幕外战斗 |

## Performance Implications
- **CPU**: 无 Camera 计算开销（无平滑插值、无边界检查）
- **Memory**: 每个房间一个静态场景，无需流式加载
- **Load Time**: 房间切换为场景替换，< 100ms
- **Network**: 不适用

## Migration Plan
无 — 新项目初始决策。未来如需滚动模式：
1. 启用 Camera2D smoothing
2. 扩展 limit 范围到房间实际尺寸
3. 房间场景尺寸从 1280×720 扩展到自定义大小
4. HUD 保持 CanvasLayer 不变

## Validation Criteria
1. Camera2D position 锁定在房间中心 (640, 360)
2. Camera2D smoothing 完全禁用
3. 玩家和敌人不可移出 [0, 1280] × [0, 720] 范围
4. HUD 在 CanvasLayer 中，不随 Camera 移动
5. 1280×720 在 1920×1080 和 2560×1440 显示器上 letterboxing 正确

## Related Decisions
- ADR-001: GDScript as Primary Language
- ADR-003: Signal Bus (room_entered / room_cleared 信号触发 HUD 更新)
