# Session State — 格斗萌主

*Last updated: 2026-06-17*

## Current Phase
UE5 Editor 启动验证 ✅

## Engine
- **Engine**: Unreal Engine 5.7.4
- **Language**: C++ (核心) + Blueprint (内容)
- **Approach**: 2.5D (固定侧视相机)
- **Project Path (local)**: `src_ue5/`
- **Project Path (cloud)**: `/root/project/FirstGame/`
- **Cloud Server**: connect.westd.seetacloud.com:51280 (RTX 5090, 754GB RAM)

## ✅ 已完成

### UE5 代码编译
- [x] 48 个 C++ 文件编译通过（修复 20+ 错误）
- [x] `libUnrealEditor-FirstGame.so` 编译链接成功
- [x] Editor 成功识别并加载 FirstGame 模块

### VNC + GPU 显示方案
- [x] **VirtualGL 安装配置** — 拦截 OpenGL 重定向到 GPU
- [x] **TigerVNC** — 替代 TightVNC，支持 GLX
- [x] **NVIDIA Vulkan ICD 注册** — 修复缺失的 nvidia_icd.json
- [x] **vglrun 验证** — `OpenGL vendor: NVIDIA Corporation, renderer: RTX 5090`

### Editor 启动
- [x] **Engine 初始化成功** — 18.9 秒内完成
- [x] **UObject 系统** — 563 对象加载
- [x] **Slate UI** — 用户创建成功
- [x] **Vulkan RHI** — API 1.1 初始化成功
- [x] **WorldPartition** — 初始化完成
- [x] **Audio** — Audio Device Manager 初始化
- [x] **Map Load** — 加载 Untitled_1 地图
- [x] **Content Browser 插件** — 加载完成

## ⚠️ 当前问题

### Vulkan Swapchain 失败
- Editor 启动 18.9 秒后在 Vulkan swapchain 创建阶段崩溃
- 位置: `VulkanUtil.cpp:847` → `VulkanSwapChain.cpp:401`
- 原因: VirtualGL 通过 OpenGL 拦截渲染，但 Vulkan swapchain 需要原生 surface presentation 支持
- TigerVNC 不提供 Vulkan presentation 能力

### 可能的解决方案
1. **x11vnc + 真实 X server (NVIDIA DRM)** — 需要正确配置 xorg.conf BusID
2. **Parsec/Moonlight** — 游戏串流方案，支持 Vulkan
3. **Software RHI** — `-nullrhi` + 软件渲染（慢但能跑）
4. **UnrealEditor-Cmd** — 命令行模式做资产创建（不需要 UI）
5. **远程桌面 (RDP/xrdp)** — 可能支持 Vulkan presentation

## 🔧 启动命令

```bash
# 启动 TigerVNC
su - ue5 -c 'vncserver :3 -geometry 1920x1080 -depth 24 -SecurityTypes VncAuth'

# 启动 UE5 Editor (Vulkan RHI + VirtualGL + NVIDIA)
su - ue5 -c 'export DISPLAY=:3 && export VGL_GLXVENDOR=nvidia && \
  /opt/VirtualGL/bin/vglrun /root/autodl-tmp/UE5/Engine/Binaries/Linux/UnrealEditor \
  /root/project/FirstGame/FirstGame.uproject -stdout -nosplash'

# UnrealEditor-Cmd (命令行模式，不需要显示)
su - ue5 -c '/root/autodl-tmp/UE5/Engine/Binaries/Linux/UnrealEditor-Cmd \
  /root/project/FirstGame/FirstGame.uproject -stdout -exec="..." -game'
```

## VNC 连接信息
- **端口**: 5903 (TigerVNC :3)
- **密码**: ue5dev
- **SSH 隧道**: `ssh -L 5903:localhost:5903 -p 51280 root@connect.westd.seetacloud.com`

## Next Steps
1. 解决 Vulkan swapchain 问题（推荐 RDP 或 Parsec 方案）
2. 创建 Blueprint 资产（DataAsset、BehaviorTree、AnimMontage）
3. 导入 AI 生成的角色 sprite
4. 创建测试 Level 验证基础玩法
