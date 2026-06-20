# UE5 Headless 资产导入方案

> **状态**: 待验证（2026-06-19 更新）
> **适用**: GPU 服务器 Vulkan 显示不可用时的备选方案

## 问题回顾

- Vulkan Swapchain 失败：容器环境无 DRM 访问
- UE5.7 仅支持 Vulkan RHI，无法回退 OpenGL
- 之前尝试：Game 模式 + Python → MaterialInstance 断言崩溃
- 之前尝试：Editor 模式 + `-ExecutePythonScript` → 脚本不执行

## 新方案：Editor 模式 + `-RunPythonScript`

### 关键发现

之前的错误：
1. 用了旧语法 `-ExecutePythonScript`（UE4/早期 UE5 的 PythonEditorScript 插件语法）
2. 在 Game 模式下运行（`GIsEditor=false` 导致材质断言崩溃）

正确方式：
- **Editor 模式**（默认）+ `-RunPythonScript`（UE5 标准语法）
- `-unattended` 抑制 UI 弹窗
- `-nullrhi` 跳过渲染（不需要 Vulkan surface）
- `-stdout -FullStdOutLogOutput` 完整日志

### 命令

```bash
# 在 GPU 服务器上执行
su - ueuser -c '
/root/autodl-tmp/ue5/Engine/Binaries/Linux/UnrealEditor-Cmd \
  /root/autodl-tmp/project/first-game/src_ue5/FirstGame.uproject \
  -RunPythonScript="/root/autodl-tmp/project/first-game/src_ue5/Content/Python/import_skeletal_mesh.py" \
  -stdout \
  -FullStdOutLogOutput \
  -unattended \
  -nosplash \
  -nullrhi
'
```

### 为什么这个应该能工作

| 之前的问题 | 现在的解决方案 |
|---|---|
| Game 模式 GIsEditor=false → 断言崩溃 | Editor 模式 GIsEditor=true → 断言通过 |
| `-ExecutePythonScript` 无效 | `-RunPythonScript` 是 UE5 正确语法 |
| Vulkan 渲染崩溃 | `-nullrhi` 跳过渲染 |
| UI 弹窗阻塞 | `-unattended` 自动模式 |

### 风险

1. `-nullrhi` + Editor 模式可能导致某些 Editor 子系统不可用
2. AssetTools 的 FBX 导入可能依赖渲染上下文（概率低）
3. 如果仍然崩溃，需要尝试 VirtualGL/TurboVNC 方案

### 备选方案

如果 `-RunPythonScript + -nullrhi` 不可行：

1. **VirtualGL + TurboVNC**
   - VirtualGL 拦截 GL/Vulkan 调用，重定向到 GPU
   - TurboVNC 提供虚拟 X server
   - 与 TigerVNC 不同，专为 3D 加速设计

2. **UE5 Build Machine 模式**
   ```bash
   UnrealEditor-Cmd ... -buildmachine -nullrhi
   ```
   CI/CD 模式，可能更适合 headless 场景

3. **自定义 Commandlet**
   - 编写 C++ UCommandlet 子类
   - 完全控制导入逻辑
   - 不依赖 Python 插件

## 导入脚本

### 1. 导入骨骼网格体 + 材质 + 贴图

见 `src_ue5/Content/Python/import_skeletal_mesh.py`（待编写）

### 2. 导入动画序列

见 `src_ue5/Content/Python/import_all_anims_final.py`（已存在）

### 3. 创建 Animation Blueprint

见 `src_ue5/Content/Python/create_abp.py`（已存在）

## 验证步骤

1. 先导入骨骼网格体：
   ```bash
   UnrealEditor-Cmd ... -RunPythonScript="import_skeletal_mesh.py" -nullrhi -unattended
   ```
2. 检查日志，确认 SKM_Huikong 导入成功
3. 导入动画：
   ```bash
   UnrealEditor-Cmd ... -RunPythonScript="import_all_anims_final.py" -nullrhi -unattended
   ```
4. 创建 ABP：
   ```bash
   UnrealEditor-Cmd ... -RunPythonScript="create_abp.py" -nullrhi -unattended
   ```
5. 运行 AnimTest 关卡验证

## 参考

- [UE5 Python API](https://docs.unrealengine.com/5.3/en-US/PythonAPI/)
- [Automating FBX Import](https://forums.unrealengine.com/t/automating-fbx-import-with-python/)
- [Headless Batch Processing](https://dev.epicgames.com/documentation/en-us/unreal-engine/)
