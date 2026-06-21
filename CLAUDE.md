# Claude Code Game Studios -- Game Studio Agent Architecture

Indie game development managed through 49 coordinated Claude Code subagents.
Each agent owns a specific domain, enforcing separation of concerns and quality.

## Technology Stack

- **Engine**: Unreal Engine 5.7
- **Language**: C++ (核心系统) + Blueprint (内容/关卡/动画/UI)
- **Version Control**: Git with trunk-based development
- **Build System**: UE5 Build System (UBT) + UnrealBuildTool
- **Asset Pipeline**: UE5 Content Browser (meshes, textures, animations, materials)

> **Note**: Using UE5 specialist agent set — unreal-specialist, unreal-cpp-specialist, unreal-gas-specialist, unreal-umg-specialist, unreal-ai-specialist.

## Project Structure

@.claude/docs/directory-structure.md

## Engine Version Reference

@docs/engine-reference/unreal/VERSION.md

## Technical Preferences

@.claude/docs/technical-preferences.md

## Coordination Rules

@.claude/docs/coordination-rules.md

## Collaboration Protocol

**User-driven collaboration, not autonomous execution.**
Every task follows: **Question -> Options -> Decision -> Draft -> Approval**

- Agents MUST ask "May I write this to [filepath]?" before using Write/Edit tools
- Agents MUST show drafts or summaries before requesting approval
- Multi-file changes require explicit approval for the full changeset
- No commits without user instruction

See `docs/COLLABORATIVE-DESIGN-PRINCIPLE.md` for full protocol and examples.

> **Game: 格斗萌主** — Q版3D第三人称动作游戏（类崩坏3）。引擎 UE5.7，概念已确立。
> 继续开发使用: `/brainstorm` `/map-systems` `/design-system` `/prototype` `/dev-story`

## Coding Standards

@.claude/docs/coding-standards.md

## Context Management

@.claude/docs/context-management.md

## Project Memory

> 项目级记忆，每次会话自动加载。详见 `.claude/memory/` 目录。

- **项目概况**: `.claude/memory/first-game-project.md`
- **本地 GPU 服务器**: `.claude/memory/first-game-on-local-gpu-server.md`
- **UE5 渲染方案**: `.claude/memory/ue5-local-gpu-rendering-breakthrough.md`
- **UE5 Python API 限制**: `.claude/memory/ue57-python-api-limitations.md`
- **剩余任务清单**: `.claude/memory/first-game-remaining-blockers.md`

### 关键记忆摘要

**睡眠策略**: 本机性能很好 (Xeon + RTX 3090)，使用 `sleep 10` + 循环检查，不用长 sleep。

**AnimBP 状态机**: 首选 C++ `UMonsterAnimInstance` 类（`Source/FirstGame/Public/Anim/`），完全绕过 Blueprint AnimGraph 编辑。

**UE5 Editor GUI**: 可用，通过 `DISPLAY=:99` + `VK_ICD_FILENAMES=.../nvidia_icd.json` 启动。

**Candy Dungeon 4 怪物**: 完整管线完成（52 uassets + C++ 动态材质 + 编辑器验证），分支 `monster-3d-pipeline`。

## UE5 MCP 远程编辑器控制

> 通过 MCP 协议远程操控 GPU 服务器上的 UE5.7 编辑器。
> 所有 Agent 可直接调用 MCP 工具或通过 `ue-mcp-specialist` 代理执行编辑器操作。

| 项目 | 值 |
|------|-----|
| 服务器 | `172.25.0.86:3000` |
| 协议 | MCP Streamable HTTP |
| MCP Agent | `ue-mcp-specialist`（23+ 工具域 / 全部 action） |
| 技能 | `/ue5-mcp`（手动调用） |
| 连接 | Session 自动管理，超时 1 小时需重连 |

### 能力概览

| 领域 | 说明 |
|------|------|
| `control_actor` | Actor 生成/删除/变换/材质/碰撞/标签 |
| `control_editor` | PIE 启停/截图/相机/控制台命令 |
| `inspect` | 对象/属性/场景/性能/运行时数据查询 |
| `manage_asset` | 资产搜索/导入/材质实例/LOD |
| `manage_blueprint` | 蓝图创建/节点编辑/变量/函数/连线/UI |
| `manage_level` | 关卡加载/光照/World Partition/Data Layer |
| `build_environment` | 地形/水体/天气/后处理/Spline/Lumen |
| `manage_geometry` | 程序化几何体/布尔运算/Nanite |
| `animation_physics` | 动画蓝图/状态机/Control Rig/蒙太奇 |
| `manage_effect` | Niagara 粒子/GPU 模拟 |
| `manage_character` | 角色蓝图/移动/攀爬/脚步 |
| `manage_combat` | 武器/连击/格挡/弹道 |
| `manage_ai` | 行为树/黑板/EQS/导航 |
| `manage_gas` | Gameplay Ability / Effect / Attribute Set |
| `manage_networking` | 复制/RPC/增强输入/游戏模式 |
| `manage_sequence` | Sequencer 过场/关键帧 |
| `system_control` | Python 脚本/UBT 构建/Insights 性能分析 |

> **Agent 协作方式**: 当你需要操作 UE 编辑器时（生成 Actor、截图、编辑蓝图、运行 PIE），委托给 `ue-mcp-specialist`。先 initialize 获取 sessionId，再按需调用工具。截图以 base64 内联返回，可直接分析。

### 自动验证触发规则

> MCP 服务器可用时，下列改动**必须**触发自动验证；MCP 服务器不可用时，
> 记录到 `production/qa/deferred-mcp-verifications.md` 并在下次服务器上线后补验。

| 改动路径 | 自动验证动作 | 委托给 |
|---|---|---|
| `src_ue5/Content/Materials/**` | 加载关卡 → 截图 → 与基线对比 | `ue-mcp-specialist` |
| `src_ue5/Content/Maps/**` | 加载关卡 → 多视角截图 | `ue-mcp-specialist` |
| `src_ue5/Content/UI/**` 或 UMG Blueprint | 打开 Widget → 截图 | `ue-mcp-specialist` |
| `src_ue5/Source/FirstGame/**`（C++ 改动） | `run_ubt` 构建 → PIE 启动 → 截图 + `get_performance_stats` | `ue-mcp-specialist` |
| GAS / Combat / AI 相关 Blueprint | spawn 测试角色 → PIE → `runtime_report` 验证属性 | `ue-mcp-specialist` |
| Niagara / 后处理配置 | PIE → 截图 + `get_performance_stats` 抓帧时间 | `ue-mcp-specialist` |

**验证闭环**：截图/数据存到 `production/qa/evidence/mcp-[story-id]-[date].md`，
作为对应 story 的 test evidence。

**降级策略**：MCP 服务器 HTTP 非 200 或 5 秒超时 → 视为不可用 →
写入 deferred 文件 → 不阻塞开发流程，但在 story-done 时提示补验。

## 图片/美术资产生成

> 使用百炼 CLI (`bailian`) 调用通义万相生成游戏美术资产。

### CLI 安装
```bash
pip install git+https://github.com/fevin/bailian-cli.git
```

### 模型选择（根据场景）

| 场景 | 推荐模型 | 说明 |
|------|----------|------|
| 角色概念图（单张高质量） | `wanx2.1-t2i-plus` | 质量更高，适合关键视觉定稿 |
| 批量 sprite 帧（速度优先） | `wanx2.1-t2i-turbo` | 速度快，适合批量生成动画帧 |
| 风格参考/探索 | `wanx2.1-t2i-turbo` | 快速迭代多个方案 |

> **wan 2.7 系列**: 当 `wanx2.7-t2i-turbo/plus` 上线后，优先切换到 2.7 系列。
> 切换方式: `bailian image -m wanx2.7-t2i-turbo --prompt "..."`

### 角色 sprite 生成流程
1. 用 `wanx2.1-t2i-plus` 生成角色概念图（1张）
2. 以概念图为参考，用 `wanx2.1-t2i-turbo` 批量生成各动作帧
3. 裁剪去背景 → 放入 `assets/art/` 目录
4. 命名规范: `char_[name]_[action]_[frame].png`

### 环境变量
```bash
export DASHSCOPE_API_KEY="你的key"
```
