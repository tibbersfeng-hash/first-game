# ADR-001: C++ as Primary Language

- **Status**: Accepted
- **Date**: 2026-06-17 (migrated from Godot/GDScript)
- **Engine**: Unreal Engine 5.6

## Context

格斗萌主是一款 2.5D 横版格斗闯关游戏。原方案使用 Godot + GDScript，现迁移至 UE5。需要确定主语言策略。

## Decision

**C++ 为核心系统语言，Blueprint 为内容/关卡语言。**

### 分工原则

| 层级 | 语言 | 理由 |
|------|------|------|
| 核心战斗逻辑 | C++ | 性能敏感，帧精确判定 |
| 状态机/系统框架 | C++ | 复杂逻辑，需要编译期检查 |
| GAS 技能定义 | C++ 基类 + BP 子类 | 框架用 C++，具体技能用 BP 快速迭代 |
| 关卡配置 | Blueprint | 设计师友好 |
| UI 逻辑 | Blueprint (UMG) | 可视化编辑 |
| 动画蓝图 | Blueprint | 动画状态机可视化 |
| AI Behavior Tree | Blueprint | 可视化编辑 |

### 命名规范

- C++ 类: `A` (Actor), `U` (Object/Component), `F` (Struct), `E` (Enum) 前缀 + PascalCase
- Blueprint 资产: `BP_` 前缀 + PascalCase
- 数据资产: `DA_` 前缀
- 变量: camelCase

## Consequences

- ✅ 核心系统有编译期类型安全
- ✅ 性能关键路径零开销
- ✅ Blueprint 保持快速迭代能力
- ⚠️ C++ 编译时间较长（使用 Live Coding 缓解）
- ️ 需要 C++ 开发环境（Visual Studio / Rider）
