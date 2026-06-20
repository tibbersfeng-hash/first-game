---
name: first-game-remaining-blockers
description: 格斗萌主 P0 原型剩余阻塞事项和后续任务清单
metadata: 
  node_type: memory
  type: project
  originSessionId: 43872f51-7434-4cad-a99a-099c5e3aeda2
---

## 当前状态 (2026-06-21 最终)

### ✅ 已完成

**糖果地牢 4 怪物完整管线**:
1. 混元3D AI 生成 4 怪物模型 + 6 绑骨动作/怪物
2. Blender 批量后处理 (tools/blender_batch_fix_monsters.py)
3. UE5 headless 导入 (init_unreal.py env hooks) — **52 uassets**
4. C++ ABaseEnemy::ConfigureMonsterAssets() 自动加载 mesh/AnimBP + 动态彩色材质
5. 可视化验证 ✅ (Xvfb :99, 4 个 ABaseEnemy 实例可见截图确认)
6. UE5 Editor GUI 可用 ✅ (Xvfb + Vulkan 渲染)
7. Git 分支 `monster-3d-pipeline` 已推送（10 commits）

### 剩余任务（需 GUI 编辑器操作）

| 任务 | 优先级 | 阻塞原因 |
|------|--------|---------|
| ~~AnimBP 状态机~~ | ~~P1~~ | **✅ 已用 C++ MonsterAnimInstance 解决** |
| NPR 完整参数 | P1 | 需自定义父材质 (2-tone/Rim/Outline) |
| LOD 关联 | P2 | 无 Python API 导入 LOD source |
| 战斗整合测试 | P1 | 需要运行时实机验证 |
| P0 评审 | P1 | 依赖上述完成 |

### UE5.7 Python API 已知限制

- `MaterialInstanceConstant.set_vector_parameter_value` 不存在
- `SkeletalMesh.set_editor_property('materials', [mi])` 类型不匹配
- `-RunPythonScript` 在 headless 模式下不执行
- 必须用 `init_unreal.py` + env hooks 触发脚本
- `unreal.load_class(None, "/Script/Module.ClassName")` 加载 C++ 类

### AnimBP 状态机解决方案（首选）

**问题**: UE5 Python API 无法编辑 AnimGraph（Blueprint 状态机）

**首选方案**: 创建 C++ `UAnimInstance` 子类（`MonsterAnimInstance`）
- ✅ 完全绕过 Blueprint AnimGraph 编辑
- ✅ 可测试、可调试、可版本控制
- ✅ 性能优于 Blueprint
- ✅ 已实现：`Source/FirstGame/Public/Anim/MonsterAnimInstance.h/cpp`

### 环境备忘

- UE5: `/home/vipuser/ue5/`
- 渲染: `DISPLAY=:99` (Xvfb), `VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/nvidia_icd.json`
- Git: `git@github.com:tibbersfeng-hash/first-game.git`
- **Sleep 策略**: 10s sleep + 多次检查（本机性能很好，不用长 sleep）
