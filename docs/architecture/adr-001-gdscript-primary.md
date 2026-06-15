# ADR-001: GDScript as Primary Language

## Status: Accepted

## Context

格斗萌主是一个Q版2D横版格斗闯关游戏，使用 Godot 4.6.3 引擎。需要选择主开发语言。Godot 支持三种语言：GDScript、C# 和 GDExtension(C++)。

## Decision

使用 **GDScript** 作为主语言，C# 仅用于未来性能关键模块的备选方案，GDExtension(C++) 仅在 GDScript 和 C# 均无法满足性能需求时考虑。

## Considered Options

### Option A: GDScript (Selected)
**Pros:**
- 与 Godot 编辑器深度集成，即改即见
- 语法简洁，迭代速度快（对独立开发者最重要）
- 社区资源丰富，教程和示例以 GDScript 为主
- 内置类型系统（4.x）提供类型安全
- 信号/节点/资源系统原生支持

**Cons:**
- 性能不如 C#/C++（但2D横版格斗场景对性能要求可控）
- 缺少 C# 的 LINQ/async 等高级特性
- 重构大型项目时不如静态语言安全

### Option B: C#
**Pros:**
- 性能优于 GDScript（约2-5x）
- 静态类型，重构更安全
- .NET 生态丰富

**Cons:**
- 需要额外安装 .NET SDK
- 与 Godot 编辑器集成不如 GDScript 紧密
- 迭代速度慢（编译→运行 vs 即改即见）
- 社区 GDScript 示例需翻译
- 增加项目复杂度（独立开发者维护两种语言）

### Option C: Mixed GDScript + C#
**Pros:**
- 核心逻辑用 C#，快速原型用 GDScript
- 性能热点可用 C# 优化

**Cons:**
- 两种语言增加认知负担
- GDScript↔C# 互操作有陷阱（类型转换、信号绑定）
- 调试跨语言调用困难
- 独立开发者维护两套代码不现实

## Decision Rationale

1. **迭代速度 > 绝对性能**：格斗游戏手感调优需要频繁改动帧数据/缓冲窗口，GDScript 的即改即见大幅缩短迭代周期
2. **2D 横版性能可控**：非开放世界3D，10 个敌人 + 粒子特效在 GDScript 下完全可达到 60fps
3. **独立开发者精力有限**：维护一种语言比两种高效得多
4. **性能逃生舱**：如后期确实遇到性能瓶颈，可用 GDExtension 针对性优化单个模块，不需要整体迁移

## Consequences

- 所有游戏逻辑使用 GDScript 编写
- 性能预算需更严格（Player update < 1ms, 10 enemies < 3ms）
- 热点函数需用性能分析器监控
- 未来如需 C# 性能优化，通过 GDExtension 接口隔离
