# ADR-007: State Machine Pattern for Entity States

## Status
Accepted

## Date
2026-06-15

## Context

### Problem Statement
格斗萌主的所有有状态实体（玩家、敌人、Boss）需要管理多种行为状态（idle、移动、攻击阶段、受击硬直、闪避等）。原型中使用 `_is_attacking` 布尔变量 + `if/elif` 链隐式管理状态，随着状态增加，代码变得难以维护和扩展。

### Constraints
- ADR-006 要求所有状态时序基于整数帧计数（`frame_in_state`）
- 状态转换必须在 `_physics_process` 中同步执行
- 同一实体可同时需要多个状态机（如移动状态机 + 攻击状态机）
- 需要支持帧步进调试（显示当前状态 + 帧数）

## Decision

使用 **StateMachine + State 模式**管理所有实体状态。

- `State`（Node 基类）：定义 `enter()/exit()/physics_update()/handle_input()` 虚方法
- `StateMachine`（Node）：持有当前状态，管理 `frame_in_state`，处理转换
- 状态是 StateMachine 的子节点，通过 `get_children()` 自动收集
- `transition_to(state_name)` 重置 `frame_in_state = 0` 并调用 `enter/exit`

## Consequences

### Positive
- **显式状态** — 每个状态独立类，职责清晰
- **帧精确** — `frame_in_state` 整数计数，符合 ADR-006
- **可测试** — 状态逻辑可脱离场景树单元测试
- **可扩展** — 新增状态只需添加子节点

### Negative
- **节点数量增加** — 每个状态是一个 Node
- **转换延迟** — 需要字符串查找状态名

## Related Decisions
- ADR-006: Frame-Based Combat Timing (frame_in_state 是帧计数器)
- ADR-001: GDScript as Primary Language (State 基类使用 GDScript 虚方法)
