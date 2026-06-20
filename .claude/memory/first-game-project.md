---
name: first-game-project
description: 主力项目「格斗萌主」UE5.7 动作游戏的位置、技术栈与关键事实
metadata: 
  node_type: memory
  type: project
  originSessionId: 8a1d1570-807b-476b-956d-baee80014bc8
---

**主力项目：格斗萌主** — Q版 3D 第三人称动作游戏（类崩坏3）。

| 项 | 值 |
|---|---|
| 引擎 | Unreal Engine 5.7 |
| 语言 | C++（核心系统）+ Blueprint（内容/关卡/动画/UI） |
| 路径 | `/home/vipuser/first-game` |
| 仓库 | `git@github.com:tibbersfeng-hash/first-game.git`（SSH，HTTPS 被墙） |
| 阶段 | P0 原型 |

**关键目录**：`src_ue5/`（UE5 工程 + Source/FirstGame C++）、`design/`（GDD）、`docs/`（架构/ADR/engine-reference）、`.claude/`（49 个 subagent + skills + hooks）、`production/`（session-state/active.md 是会话进度 living checkpoint）。

**进入项目时加载顺序**：① `first-game/CLAUDE.md`（项目级主配置）→ ② `production/session-state/active.md`（恢复进度）→ ③ `HANDOVER.md`（迁移史）。全局人设在 `~/.claude/CLAUDE.md`。

**当前状态（2026-06-19）**：48+ C++ 文件编译通过、17 个测试文件就绪；武僧 3D 资产 + ABP 已导入；卡点是 NPR 卡通材质/描边 + 整合验证。环境与 GPU 见 [[first-game-on-local-gpu-server]]。

**注意**：`active.md` 部分段落（服务器地址、远程路径）已过时，参见 [[first-game-on-local-gpu-server]] 的纠正。
