---
name: first-game-on-local-gpu-server
description: first-game 项目与 UE5 引擎就在 Claude 当前所在的 GPU 服务器本机，无需远程连接
metadata: 
  node_type: memory
  type: project
  originSessionId: 8a1d1570-807b-476b-956d-baee80014bc8
---

Claude 当前运行的环境**本身就是 GPU 服务器**，first-game 项目、UE5 引擎、GPU 都在同一台机器上：

- **GPU**: NVIDIA RTX 3090 (24GB)，驱动 580.159.03，本机空闲可用（`nvidia-smi` 可查）
- **本机 IP**: `172.25.0.86`（也是 `.mcp.json` 里 MCP 服务器 `http://172.25.0.86:3000/mcp` 指向的地址——MCP UE5 桥就跑在本机）
- **UE5 引擎**: 装在 `/home/vipuser/ue5` 和 `/home/vipuser/UnrealEngine`
- **UE5 工程**: `/home/vipuser/first-game/src_ue5/`（FirstGame.uproject）

**关键纠正**：`production/session-state/active.md` 里写的"GPU 服务器不可用 / SSH 连接被拒 / 实例可能已停止"是**过时错误信息**。那是 Godot→UE5 迁移期间残留的旧描述（曾指向远程 `connect.nmb2.seetacloud.com:42491` + `/root/autodl-tmp/...` 路径），不再适用。资产导入、UE5 Editor/Cmd、整合测试都应在本机直接运行，不需要 SSH 到任何远程服务器。

**Why**: 之前误判"GPU 离线"导致整个 Week 3 待办（资产导入验证、NPR 材质、整合测试）被当成卡死状态搁置，实际上本机 GPU 就在眼前。

**How to apply**: 做 first-game 的 UE5 工作（编译、资产导入、Editor、截图验证、自动化测试）时，直接用本机路径与 GPU，不要去连远程服务器。需要重新核对/修订 `active.md` 中的服务器相关段落。相关环境脚本（`start_editor.sh`、`run_game_screenshot.sh`、`take_ue5_screenshot.sh`、`test_vulkan.sh`）在 `/home/vipuser/`。关联 [[first-game-project]]。
