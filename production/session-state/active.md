# Session State — 格斗萌主

*Last updated: 2026-06-18 20:38*

## Current Phase
**P0 原型 — 3D 模型已生成，UE5 导入受阻**

## 服务器信息
- **地址**: connect.nmb2.seetacloud.com:42491（已更新，旧地址已失效）
- **GPU**: NVIDIA RTX 3090 (24GB)
- **内存**: 756GB
- **UE5**: 5.7.4 (CL-51494982)
- **项目路径**: `/root/autodl-tmp/project/first-game/src_ue5/`
- **VNC**: TigerVNC :1 (端口 5901)
- **用户**: ueuser

## 方向确认
- **风格**: 3D 第三人称动漫风格化（正常比例，非 Q 版）
- **头身比**: 6.5-7.5 头身（崩坏3/原神风格）
- **引擎**: Unreal Engine 5.7.4
- **美术管线**: 方案 E（混合管线）
  - AI 3D 生成: tccli ai3d（腾讯云混元 3D）
  - 模型修整: Blender 4.x
  - 绑骨+动画: 混元绑骨（48种预设动作）
  - 渲染: UE5 NPR (Toon Shading)

## ✅ 已完成

### UE5 代码编译
- [x] 48 个 C++ 文件编译通过
- [x] `libUnrealEditor-FirstGame.so` 编译链接成功（1.3MB）

### 武僧 3D 生成 (2026-06-18 19:32)
- [x] Prompt 优化（加入 NOT chibi 等反向提示）
- [x] 文生3D 完成（model 3.1, 150k faces）
- [x] 混元绑骨完成（7 个预设动作）
- [x] Blender 分析：头身比 1:6.67 ✅
- [x] 缩放至 185cm 并导出 UE5 版本

### 服务器环境 (2026-06-18 20:38)
- [x] 云服务器连接成功（新地址）
- [x] NVIDIA Vulkan ICD 注册到标准目录
- [x] PythonScriptPlugin 启用
- [x] FBX 文件上传到服务器（原始 41MB + 重拓扑 1.7MB）
- [x] UnrealEditor-Cmd `-nullrhi` 模式可运行
- [x] x11vnc 已安装

### Blender 模型修整 (2026-06-18 22:00)
- [x] 全面拓扑分析
- [x] Voxel Remesh 重拓扑 (1.5cm) → 28,744 四边面 / 1.7MB
  - ⚠️ 丢失面部和服装细节
- [x] **手工修整** (Decimate Un-Subdivide + Collapse) → **40,007 面 / 1.5MB**
  - ✅ 保留面部特征、僧衣细节、腰带、护腕、飘带、靴子
  - ✅ 100% 三角面（Decimate 特性，UE5 可接受）
  - ✅ 法线统一 + 平滑着色
  - ✅ UV 智能重新展开
- [x] 两个版本均已上传服务器
- [ ] 面部细节可进一步手工优化（需在 Blender GUI 中操作）

### Python 脚本调查结论 (2026-06-18 21:40)
- ✅ **自动执行**: `Content/Python/init_unreal.py` 在 `-game` 模式下自动执行
- ✅ **权限修复**: `chown -R ueuser:ueuser` 项目目录
- ❌ **FBX 导入崩溃**: `MaterialInstanceConstant.cpp:163` — `GIsEditor || IsRunningCommandlet()` 断言失败
- ❌ **Editor 模式**: 不自动执行 `init_unreal.py`，`-ExecutePythonScript` 无效
-  **结论**: 需要 Editor GUI（解决 Vulkan 问题）才能完成 FBX 导入

## ❌ 阻塞问题

### Vulkan Swapchain 失败（阻塞 UE5 Editor GUI）— 已穷尽所有方案
- 错误: `vkGetPhysicalDeviceSurfacePresentModesKHR failed, VkResult=-13`
- 根因: 容器环境无法为 NVIDIA GPU 提供有效的 Vulkan 呈现表面

#### 已尝试的方案（全部失败）

| 方案 | 结果 | 原因 |
|---|---|---|
| TigerVNC + NVIDIA | ❌ | TigerVNC 不支持 Vulkan 呈现 |
| `-opengl4` | ❌ | UE5.7 已移除 OpenGL 支持 |
| `-EGL` | ❌ | 仍使用 Vulkan RHI，同样崩溃 |
| `-nullrhi` | ✅ | UnrealEditor-Cmd 可用（无 GUI） |
| Xorg + NVIDIA 驱动 | ❌ | 容器无 DRM 访问，"no screens found" |
| Xvfb + x11vnc | ❌ | Xvfb 不提供 GPU 呈现表面 |
| lavapipe（软件 Vulkan）| ❌ | 不满足 SM5 feature level 要求 |
| lavapipe + ForceFeatureLevel | ❌ | 跳过 SM5 后无可用 RHI，卡死 |
| NVIDIA + lavapipe 双 ICD | ❌ | UE5 优先选 NVIDIA，仍崩溃 |
| NVIDIA ICD 注册 | ❌ | 注册了但 Xvfb 无表面支持 |
| x11vnc 安装 | ✅ | 已安装，但底层 X 不支持 Vulkan |

#### 根本限制
- **容器环境**：无 DRM/KMS 访问（`[drm] Failed to open DRM device: -95`）
- **UE5.7 Linux**：仅支持 Vulkan RHI，无 OpenGL 回退
- **lavapipe**：不支持 UE5 所需的 SM5 feature level
- **Xvfb/TigerVNC**：无法创建 `VK_KHR_xlib_surface` 兼容的呈现表面

#### 唯一可行解
1. **平台方开启 DRM modeset** — 宿主机内核参数，需平台方操作
2. **Parsec/Moonlight** — 游戏串流方案，自带 GPU 呈现管线
3. **宿主机直接运行** — 非容器环境

### Python/FBX 导入阻塞
- **根因**: `MaterialInstanceConstant.cpp:163` 断言 `GIsEditor || IsRunningCommandlet()`
- Game 模式：Python 可执行，但材质创建崩溃（即使禁用材质导入）
- Editor 模式：`init_unreal.py` 不自动执行，`-ExecutePythonScript` 无效
- FBX 纹理已成功提取到 `.fbm/` 目录
- **解决路径**: 需要 Editor GUI 手动导入，或找到正确的 Commandlet

## 🔧 启动命令

```bash
# VNC 连接
ssh -L 5901:localhost:5901 -p 42491 root@connect.nmb2.seetacloud.com

# UnrealEditor-Cmd（无 GUI，命令行模式）
su - ueuser -c '/root/autodl-tmp/ue5/Engine/Binaries/Linux/UnrealEditor-Cmd \
  /root/autodl-tmp/project/first-game/src_ue5/FirstGame.uproject -stdout -nullrhi'

# FBX 文件位置
/root/autodl-tmp/project/first-game/src_ue5/Content/Characters/Huikong/huikong_3d.fbx
```

## 下一步
1. 解决 Vulkan 显示问题（推荐 x11vnc + NVIDIA DRM 或 Parsec）
2. 排查 Python 脚本执行问题
3. 导入武僧 3D 模型到 UE5
4. 创建测试 Level 验证基础玩法

## 📋 P0 原型计划概览

**周期**: 3 周 (21 天)
**角色**: 气功小武僧 (Huikong)
**目标**: 完成 1 个可玩角色原型 + P0 评审

| 周次 | 主题 | 主要交付物 |
|---|---|---|
| Week 1 | AI 3D 生成 + Blender 修整 | block-out 模型 FBX |
| Week 2 | 贴图 + 混元绑骨 + 基础动画 | 绑骨完成 + 4 个基础动画 |
| Week 3 | 战斗动画 + NPR + 整合 | 完整战斗动画集 + NPR 材质 + P0 评审 |

详细任务见: `production/p0-prototype/week-tasks.md`
