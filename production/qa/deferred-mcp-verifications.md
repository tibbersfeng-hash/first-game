# Deferred MCP Verifications

> MCP 服务器不可用时的验证任务记录。服务器恢复后需补验。

## 待补验任务

_（暂无待补验任务）_

---

## 格式说明

每条记录包含：
- **日期**：记录时间
- **Story ID**：相关 story
- **改动文件**：触发的文件路径
- **验证类型**：截图/runtime report/性能数据
- **状态**：待补验 / 已完成

示例：
```
### 2026-06-21 | Story: EPIC-001-S02
- **改动**: `src_ue5/Source/FirstGame/Private/Combat/CombatAbility.cpp`
- **验证类型**: PIE + runtime_report
- **状态**: ⏳ 待补验
```
