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
