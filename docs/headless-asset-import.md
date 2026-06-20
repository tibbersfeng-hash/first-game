# UE5 Headless 资产导入方案

> **状态**: ✅ 已验证（2026-06-20 更新）
> **适用**: GPU 服务器 Vulkan 显示不可用时的备选方案

## ⚠️ UE5.7 Python 脚本执行方法（重要）

### 正确的命令行参数

经过多次测试验证，UE5.7 中执行 Python 脚本的**唯一正确方法**：

```bash
UnrealEditor-Cmd <ProjectPath> -run=PythonScript -script=<script.py> -stdout -nullrhi -unattended
```

### 错误参数（不工作）

| 参数 | 状态 | 原因 |
|---|---|---|
| `-RunPythonScript=<path>` | ❌ 不工作 | UE5.7 不识别此参数，脚本不会执行 |
| `-ExecutePythonScript=<path>` | ❌ 仅 GUI 模式 | 源码要求编辑器模式，commandlet/-nullrhi 下被拒绝 |

### 源码依据

```cpp
// PythonScriptPlugin/Source/.../EditorPythonExecuter.cpp:144
const TCHAR* Match = TEXT("-ExecutePythonScript=");

// 第 184 行检查
UE_LOG(LogEditorPythonExecuter, Error, TEXT("-ExecutePythonScript cannot be used outside of the editor."));
// 第 188 行检查
UE_LOG(LogEditorPythonExecuter, Error, TEXT("-ExecutePythonScript cannot be used by a commandlet. Use -run=PythonScript instead?"));
```

### 验证日志

```
LogPythonScriptCommandlet: Display: Running Python script: /tmp/test_material_minimal.py
[成功创建 M_Huikong_Test.uasset]
```

### 完整示例命令

```bash
export DISPLAY=:99  # 或任何可用的 X server

/home/vipuser/ue5/Engine/Binaries/Linux/UnrealEditor-Cmd \
  /home/vipuser/first-game/src_ue5/FirstGame.uproject \
  -run=PythonScript \
  -script=/path/to/your_script.py \
  -stdout -nullrhi -unattended
```

### DISPLAY 环境

- ✅ **DISPLAY=:99** — Xvfb + x11vnc（端口 5902），已确认可用
- ❌ DISPLAY=:1 — TigerVNC，截图全黑
- ❌ DISPLAY=:10 — xrdp，截图全黑

> **固定使用 DISPLAY=:99** 进行所有 UE5 编辑器操作。

## 导入脚本

### 1. 导入骨骼网格体 + 材质 + 贴图

见 `src_ue5/Content/Python/import_skeletal_mesh.py`（待编写）

### 2. 导入动画序列

见 `src_ue5/Content/Python/import_all_anims_final.py`（已存在）

### 3. 创建 Animation Blueprint

见 `src_ue5/Content/Python/create_abp.py`（已存在）

## 验证步骤

> ⚠️ 所有命令使用 `-run=PythonScript -script=<path>`，不是 `-RunPythonScript`。

1. 先导入骨骼网格体：
   ```bash
   UnrealEditor-Cmd ... -run=PythonScript -script=import_skeletal_mesh.py -nullrhi -unattended
   ```
2. 检查日志，确认 SKM_Huikong 导入成功
3. 导入动画：
   ```bash
   UnrealEditor-Cmd ... -run=PythonScript -script=import_all_anims_final.py -nullrhi -unattended
   ```
4. 创建 ABP：
   ```bash
   UnrealEditor-Cmd ... -run=PythonScript -script=create_abp.py -nullrhi -unattended
   ```
5. 运行 AnimTest 关卡验证

## UE5.7 Python API 注意事项

- `unreal.ShadingModel` **不存在**（UE5.7 变更）
- `MaterialEditingLibrary.connect_material_property()` 最多接受 **3 个参数**（不是 4 个）
- 材质连接需要用 `connect_material_expression()` 或其他方式

## 参考

- [UE5 Python API](https://docs.unrealengine.com/5.3/en-US/PythonAPI/)
- [Automating FBX Import](https://forums.unrealengine.com/t/automating-fbx-import-with-python/)
- [Headless Batch Processing](https://dev.epicgames.com/documentation/en-us/unreal-engine/)
