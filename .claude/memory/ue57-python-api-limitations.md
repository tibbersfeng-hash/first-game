---
name: ue57-python-api-limitations
description: UE5.7 Python API 已知限制和踩坑记录 — 材质 flag、类型转换、废弃 API
metadata: 
  node_type: memory
  type: reference
  originSessionId: 43872f51-7434-4cad-a99a-099c5e3aeda2
---

## UE5.7 Python API 已知限制 (2026-06-20 晚更新)

### 0. FBX 导入 API 属性名陷阱

| 错误的属性名 | 正确 API |
|---|---|
| `FbxImportUI.skeletal_mesh_import_options` | `FbxImportUI.skeletal_mesh_import_data` |
| `FbxSkeletalMeshImportData.b_import_morph_targets` | 不存在（UE5.7 移除或改名） |
| `FbxSkeletalMeshImportData.import_mesh_lo_ds` | 不存在（用默认） |
| `AnimBlueprintFactory.target_skeleton` | 不存在（在创建后的 ABP 上设 `.target_skeleton`） |

### 1. 材质 Usage Flag 无法通过 Python 修改

**问题**: `UMaterial.bUsedWithSkeletalMesh` 等 usage flag 在 Python 中完全不可访问。

**已尝试的方法（全部失败）**:
- `material.set_editor_property('bUsedWithSkeletalMesh', True)` → `AttributeError: no attribute`
- `setattr(material, 'bUsedWithSkeletalMesh', True)` → 属性不存在
- `material.b_used_with_skeletal_mesh` → `AttributeError`
- 控制台命令 `MatSetSkeletalMesh` → 该命令不存在
- C++ Commandlet (`UCommandlet::Main`) → commandlet 模式资产加载 crash
- 二进制 uasset 修改 → 格式太复杂，风险高

**解决方案**: 必须在 GUI 编辑器中手动操作：
> 打开材质 → Details → Usage → 勾选 "Used With Skeletal Mesh" → Save

**根本原因**: UE5 的 `UMaterial` 中 usage flags 可能是通过非 UPROPERTY 机制序列化的（打包在 uint32 Usage 位域中），Python 反射无法访问。

### 2. Python 类型转换陷阱

| API | 期望类型 | 常见错误 |
|---|---|---|
| `MaterialExpressionConstant3Vector.constant` | `unreal.LinearColor(r,g,b,a)` | ❌ `unreal.Vector(x,y,z)` |
| `DirectionalLightComponent.LightColor` | `unreal.Color(R,G,B)` (0-255) | ❌ `unreal.LinearColor(r,g,b,a)` (0-1) |
| `MaterialEditingLibrary.connect_material_expressions(from, pin, to, pin)` | `to` 必须是 `MaterialExpression` | ❌ 传 `Material` 对象 |
| `ParseCommandLine(*Params, ...)` | `*Params` (TCHAR*) |  `Params` (FString&) |

### 3. UE5.7 废弃 API

| 废弃 API | 替代方案 |
|---|---|
| `EditorLevelLibrary.get_all_level_actors()` | Editor Actor Utilities Subsystem |
| `EditorLevelLibrary.save_current_level()` | Level Editor Subsystem |
| `unreal.ShadingModel` 枚举 | 不存在于 UE5.7 |

### 4. UE5.7 不存在/变更的 API

- `unreal.ShadingModel` — 不存在
- `MaterialEditingLibrary.connect_material_property()` — 最多 3 个参数（不是 4 个）
- `DirectionalLightComponent.AtmosphereSunLight` — 属性不存在
- `UPackageTools.SaveDirtyPackages()` — 方法不存在
- `AnimInstance.is_playing()` — 方法不存在，用 `is_active()` 替代

### 5. Commandlet 模式注意事项

- `UnrealEditor-Cmd` 的 `-run=PythonScript` 使用 `-nullrhi`（无渲染）
- 自定义 Commandlet 首次运行可能 Signal 11 crash，重试通常可解决
- Commandlet 模式下 `LogTemp` 日志会输出到 stdout（与编辑器模式不同）
- Commandlet 的 `Main()` 接收的 `Params` 是 `-run=XXX` 后面的所有内容

### 6. 编辑器模式注意事项

- `-ExecutePythonScript=` 脚本中的 `unreal.log()` **不会出现在 stdout**
- `-RunPythonScript` 在 `-nullrhi -unattended` 模式下**完全不执行**（已验证无效）
- ✅ **正确做法**: 通过 `Content/Python/init_unreal.py` + 环境变量钩子触发脚本
  ```bash
  FIRSTGAME_IMPORT_MONSTERS=1 UnrealEditor-Cmd FirstGame.uproject -nullrhi -unattended
  ```
  init_unreal.py 检测 env var → import 对应的模块 → 执行 → quit
- 脚本中的 Python 错误**会**出现在 stdout（`LogPython: Error:`）
- 通过文件时间戳变化验证脚本是否修改了资产
- 截图通过 `HighResShot` 命令保存到 `Saved/Screenshots/LinuxEditor/`

### 7. UE5.7 启动阻塞修复

- `UAchievementManagerComponent`, `UDialogueManagerComponent`, `UEquipmentManagerComponent`, `USkillTreeComponent` 构造函数中的 `SetIsReplicated(true)` 会触发 UE5.7 ensure 失败
- **修复**: 全部替换为 `SetIsReplicatedByDefault(true)`（UE5.7 的正确方式）
