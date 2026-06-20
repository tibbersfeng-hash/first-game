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
