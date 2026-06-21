---
name: verify-huikong-anim
description: "验证慧空角色动画状态机 — Idle/Walk/Run/Attack 动画切换，包含自动截图和图片自确认环节"
argument-hint: "[full|idle|walk|run|attack|flying]"
user-invocable: true
allowed-tools: Bash, Read, Write, Glob
model: sonnet
---

## 概述

验证慧空（Huikong）角色的动画状态机是否正常工作，包括：
- Idle 动画（默认静止）
- Walk 动画（慢走）
- Run 动画（快速跑）
- Attack 动画（攻击）
- 飞天问题检查（Z 坐标验证）

**关键特性**：包含图片自确认环节，确保视觉验证闭环。

---

## Phase 1: 解析参数

从用户输入确定测试模式：

- `full` → 完整测试所有动画状态（默认）
- `idle` → 仅测试 Idle 动画
- `walk` → 仅测试 Walk 动画
- `run` → 仅测试 Run 动画
- `attack` → 仅测试 Attack 动画
- `flying` → 仅检查飞天问题

如果未提供参数，默认为 `full`。

---

## Phase 2: 环境检查

### 2.1 检查 MCP 服务器

```bash
curl -s -m 5 --noproxy '*' -X POST http://172.25.0.86:3000/mcp \
  -H "Content-Type: application/json" \
  -H "X-MCP-Capability-Token: c74a40cde871789c0b27984019fc58e4" \
  -d '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-03-26","capabilities":{},"clientInfo":{"name":"health-check","version":"1.0"}}}'
```

- HTTP 200 + sessionId → MCP 可用
- 其他 → 走降级策略（本地截图）

### 2.2 检查编辑器状态

```bash
ps aux | grep "UnrealEditor.*FirstGame" | grep -v grep
```

- 编辑器运行中 → 继续
- 编辑器未运行 → 启动编辑器

### 2.3 检查测试关卡

```bash
ls -la /home/vipuser/first-game/src_ue5/Content/Maps/TestLevel_Huikong.umap 2>/dev/null
```

- 存在 → 加载关卡
- 不存在 → 创建测试关卡

---

## Phase 3: 设置测试场景

### 3.1 创建/加载测试关卡

使用 Python 脚本设置测试场景：

```bash
FIRSTGAME_HUIKONG_TEST=1 bash ~/start_editor.sh
```

或手动在编辑器 Python 控制台执行：

```python
import unreal

# 加载关卡
unreal.EditorLevelLibrary.load_level("/Game/Maps/TestLevel_Huikong")

# 设置慧空角色
huikong_mesh = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong")
huikong_abp = unreal.load_asset("/Game/Characters/Huikong/ABP/ABP_Huikong")

# 设置地面、光照、相机
# ... (详见 test_huikong_full.py)
```

### 3.2 验证场景设置

截图确认：
- ✅ 慧空角色可见
- ✅ 地面可见
- ✅ 光照正常
- ✅ 相机对准角色

---

## Phase 4: 执行动画测试

### 4.1 Idle 动画测试

1. 确保角色静止
2. 等待 3 秒
3. 截图 `Huikong_Idle.png`
4. 检查 Z 坐标 < 200

**验证标准**：
- 角色播放 Idle 动画（呼吸/待机动作）
- 角色站在地面上（Z ≈ 50）
- 无飞天现象

### 4.2 Walk 动画测试

1. 运行 PIE
2. 输入 `MoveForward 1.0`（慢走）
3. 等待 3 秒
4. 截图 `Huikong_Walk.png`
5. 检查 Z 坐标 < 200
6. 输入 `MoveForward 0` 停止

**验证标准**：
- 角色播放 Walk 动画
- 角色在地面上移动
- 无飞天现象

### 4.3 Run 动画测试

1. 输入 `MoveForward 1.0`
2. 输入 `Sprint 1`（快速跑）
3. 等待 3 秒
4. 截图 `Huikong_Run.png`
5. 检查 Z 坐标 < 200
6. 停止输入

**验证标准**：
- 角色播放 Run 动画
- 移动速度明显快于 Walk
- 无飞天现象

### 4.4 Attack 动画测试

1. 确保角色静止或移动中
2. 输入 `LightAttack`
3. 等待 2 秒
4. 截图 `Huikong_Attack.png`
5. 检查 Z 坐标 < 200

**验证标准**：
- 角色播放攻击动画
- 攻击动作流畅
- 无飞天现象

---

## Phase 5: 图片自确认环节 

### 5.1 自动截图收集

所有截图保存到：
```
/home/vipuser/first-game/production/qa/evidence/
├── Huikong_Idle_{timestamp}.png
├── Huikong_Walk_{timestamp}.png
├── Huikong_Run_{timestamp}.png
├── Huikong_Attack_{timestamp}.png
└── huikong_test_report.json
```

### 5.2 图片验证清单

使用 Read 工具查看每张截图，确认：

| 检查项 | Idle | Walk | Run | Attack |
|--------|------|------|-----|--------|
| 角色可见 | ✅ | ✅ | ✅ | ✅ |
| 地面可见 | ✅ | ✅ | ✅ | ✅ |
| 光照正常 | ✅ | ✅ | ✅ | ✅ |
| 动画播放 | ✅ | ✅ | ✅ | ✅ |
| 无飞天 | ✅ | ✅ | ✅ | ✅ |

### 5.3 飞天问题专项检查

从日志或截图中检查：
- 角色 Z 坐标是否 > 200
- 角色是否站在地面上
- 重力是否正常工作

---

## Phase 6: 生成验证报告

### 6.1 自动生成报告

```markdown
# 慧空动画验证报告

**日期**: {timestamp}
**测试模式**: {full|idle|walk|run|attack|flying}
**执行者**: {agent-name}

## 测试结果

| 测试项 | 状态 | 截图 | Z 坐标 | 备注 |
|--------|------|------|--------|------|
| Idle 动画 | ✅/❌ | [链接] | {z} | {notes} |
| Walk 动画 | ✅/❌ | [链接] | {z} | {notes} |
| Run 动画 | ✅/❌ | [链接] | {z} | {notes} |
| Attack 动画 | ✅/❌ | [链接] | {z} | {notes} |
| 飞天检查 | ✅/❌ | - | {z} | {notes} |

## 截图证据

- Idle: `production/qa/evidence/Huikong_Idle_{timestamp}.png`
- Walk: `production/qa/evidence/Huikong_Walk_{timestamp}.png`
- Run: `production/qa/evidence/Huikong_Run_{timestamp}.png`
- Attack: `production/qa/evidence/Huikong_Attack_{timestamp}.png`

## 问题汇总

{如有问题，列出具体问题}

## 结论

✅ **所有测试通过** / ⚠️ **部分测试失败**

{具体建议}
```

### 6.2 保存到文件

```bash
/home/vipuser/first-game/production/qa/evidence/huikong_anim_verify_{timestamp}.md
```

---

## Phase 7: 后续行动

### 7.1 如果测试通过

- 标记验证完成
- 更新 session-state/active.md
- 提交测试截图和报告

### 7.2 如果测试失败

- 记录具体问题
- 分析可能原因
- 提供修复建议
- 标记需要重新验证

### 7.3 降级策略

如果 MCP 服务器不可用：
- 使用本地 Python 截图
- 手动在 VNC 上验证
- 记录到 deferred-mcp-verifications.md

---

## 附录：Python 测试脚本

完整测试脚本位于：
```
/home/vipuser/first-game/src_ue5/Content/Python/test_huikong_full.py
```

触发方式：
```bash
FIRSTGAME_HUIKONG_TEST=1 bash ~/start_editor.sh
```

---

## 示例用法

```bash
# 完整测试
/verify-huikong-anim full

# 仅测试 Walk 动画
/verify-huikong-anim walk

# 仅检查飞天问题
/verify-huikong-anim flying
```

---

## 相关文档

- ABP_Huikong 动画蓝图：`src_ue5/Content/Characters/Huikong/ABP/ABP_Huikong.uasset`
- 慧空角色 Mesh：`src_ue5/Content/Characters/Huikong/Mesh/SKM_Huikong.uasset`
- 飞天问题修复：`src_ue5/Source/FirstGame/Private/Characters/BaseEnemy.cpp`
- 验证报告目录：`production/qa/evidence/`
