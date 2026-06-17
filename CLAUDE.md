# Claude Code Game Studios -- Game Studio Agent Architecture

Indie game development managed through 49 coordinated Claude Code subagents.
Each agent owns a specific domain, enforcing separation of concerns and quality.

## Technology Stack

- **Engine**: Godot 4.6.3
- **Language**: GDScript (主语言) + C# (可选性能模块)
- **Version Control**: Git with trunk-based development
- **Build System**: Godot Editor export
- **Asset Pipeline**: Godot import (sprites, audio, shaders)

> **Note**: Using Godot specialist agent set — godot-specialist, godot-gdscript-specialist, godot-shader-specialist, godot-gdextension-specialist.

## Project Structure

@.claude/docs/directory-structure.md

## Engine Version Reference

@docs/engine-reference/godot/VERSION.md

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

> **Game: 格斗萌主** — Q版2D横版格斗闯关游戏（类DNF）。引擎已配置，概念已确立。
> 继续开发使用: `/brainstorm` `/map-systems` `/design-system` `/prototype` `/dev-story`

## Coding Standards

@.claude/docs/coding-standards.md

## Context Management

@.claude/docs/context-management.md

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
