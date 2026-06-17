# 服务器工具集

> RTX 5090 服务器 + Parsec 远程桌面 + AI 自主开发

---

## 🎯 开发模式

```
┌─────────────────────────────────────────────────┐
│              GPU 服务器 (RTX 5090)               │
│                                                 │
│  ┌──────────────┐    ┌───────────────────────┐  │
│  │ SSH 命令行   │    │ Parsec 远程桌面       │  │
│  │ (AI 开发)    │    │ (人工查看/操作 UE5)   │  │
│  └──────────────┘    └───────────────────────┘  │
│         ↓                       ↓               │
│  ┌─────────────────────────────────────────┐   │
│  │         UE5.7 Editor + 项目代码         │   │
│  └─────────────────────────────────────────┘   │
│                    ↓                            │
│           RTX 5090 GPU 渲染                     │
└─────────────────────────────────────────────────┘
```

**两条并行通道**：
- **AI（Claude）**：通过 SSH 命令行编辑代码、编译、测试、截图
- **人类（你）**：通过 Parsec 远程桌面实时查看、操作 UE5 Editor

---

## 📋 工具清单

### 🏆 推荐：Parsec 方案（远程桌面）

#### 1. server-init-parsec.sh — 服务器初始化
```bash
# 在新 GPU 服务器上运行（一次性）
./tools/server-init-parsec.sh
```

**功能**：
- 安装 XFCE4 桌面环境（轻量）
- 安装 Parsec 服务端
- 配置虚拟显示器（VirtualGL + dummy driver）
- 安装基础工具 + ffmpeg
- 创建项目目录
- 生成辅助脚本

**详细指南**：👉 [PARSEC-SETUP.md](./PARSEC-SETUP.md)

#### 2. 辅助脚本
```bash
/data/parsec-status.sh    # 查看 Parsec 状态和配对码
/data/display-start.sh    # 启动虚拟显示
/data/display-stop.sh     # 停止虚拟显示
```

---

### 🎬 备用：视频录制方案

如果不想装 Parsec，也可以用视频录制方式查看效果：

#### 3. server-init.sh — 服务器初始化（无桌面版）
```bash
./tools/server-init.sh
```

#### 4. quick-record.sh — 快速录制游戏视频
```bash
# 录制 30 秒（默认）
./tools/quick-record.sh

# 录制 60 秒，指定文件名
./tools/quick-record.sh 60 combat-test
```

---

## 🚀 快速开始（Parsec 方案）

### 服务器端
```bash
# 1. SSH 登录新服务器
ssh root@your-server

# 2. 克隆项目
git clone git@github.com:tibbersfeng-hash/first-game.git
cd first-game/tools

# 3. 运行初始化
./server-init-parsec.sh

# 4. 获取配对码
/data/parsec-status.sh
```

### 客户端（你的电脑）
```bash
# 1. 安装 Parsec
# 下载: https://parsec.app/downloads

# 2. 登录账号

# 3. 添加远程机器
#    Add Machine → 输入 IP + 配对码

# 4. 连接
#    Connect → 看到 XFCE 桌面 → 启动 UE5
```

---

## 📊 性能对比

| 方案 | 延迟 | 画质 | 交互性 | 推荐度 |
|------|------|------|--------|--------|
| **Parsec** | <10ms | 60fps | ✅ 实时操作 | ⭐⭐⭐⭐⭐ |
| 视频录制 | N/A | 30fps | ❌ 仅观看 | ⭐⭐⭐ |

---

## 🔧 常用命令

```bash
# === SSH 操作（AI 开发）===
ssh root@your-server
cd /data/project/FirstGame
# 编辑代码、编译、测试...

# === Parsec 操作（人工查看）===
# 通过 Parsec 连接远程桌面
# 在桌面中打开终端
cd /data/project/FirstGame
/data/UE5/Engine/Binaries/Linux/UnrealEditor FirstGame.uproject

# === 视频录制（备用）===
/data/quick-record.sh 60 demo
scp root@server:/tmp/demo.mp4 ./
```

---

## 📚 文档

- [Parsec 设置详细指南](./PARSEC-SETUP.md)
- [视频录制方案说明](./VIDEO-RECORDING.md)
- [故障排查](./TROUBLESHOOTING.md)

---

## 📝 备注

- 服务器建议配置：300GB+ 存储、静态 IP
- Parsec 个人版免费
- 可同时使用 SSH + Parsec（互不干扰）
- UE5 需要单独安装（脚本不会自动下载）
- NVIDIA 驱动已安装（CUDA 12.8）
- 至少 300GB 存储空间

---

### 2. quick-record.sh — 快速录制游戏视频
```bash
# 录制 30 秒（默认）
./tools/quick-record.sh

# 录制 60 秒，指定文件名
./tools/quick-record.sh 60 combat-test

# 录制 120 秒
./tools/quick-record.sh 120 gameplay-demo
```

**输出**：`/tmp/gameplay-*.mp4`

**传输到本地**：
```bash
scp user@server:/tmp/gameplay-*.mp4 ./
```

**工作原理**：
```
Xvfb (虚拟显示 :99)
  ↓
VirtualGL (GPU 加速渲染)
  ↓
UE5 游戏运行
  ↓
ffmpeg 录制 → MP4
```

---

## 🎬 视频录制技术栈

```
┌─────────────────────────────────────┐
│  Xvfb :99 (虚拟帧缓冲)              │
│  - 1920x1080x24                     │
│  - 无需物理显示器                    │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  VirtualGL (GPU 加速)               │
│  - 利用 RTX 5090 渲染               │
│  - OpenGL → GPU → 帧缓冲           │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  UE5.7 Editor (游戏模式)            │
│  - -game -windowed                  │
│  - 渲染到虚拟显示                    │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  ffmpeg (录制)                      │
│  - x11grab 捕获虚拟显示             │
│  - libx264 编码                     │
│  - 30fps, CRF 23                    │
└──────────────┬──────────────────────┘
               │
               ▼
         /tmp/*.mp4
```

---

## 🔧 故障排查

### 问题 1: VirtualGL 报错 "GLX context not available"
```bash
# 解决：确保 Xvfb 启动时带 GLX 扩展
Xvfb :99 -screen 0 1920x1080x24 +extension GLX &
```

### 问题 2: ffmpeg 录制黑屏
```bash
# 检查虚拟显示是否正常运行
DISPLAY=:99 xdpyinfo | grep dimensions

# 检查 GPU 渲染是否正常
DISPLAY=:99 vglrun glxgears
```

### 问题 3: UE5 启动失败
```bash
# 检查日志
tail -f $PROJECT_DIR/Saved/Logs/FirstGame.log

# 常见原因：
# - 缺少依赖库: sudo apt install libxi6 libxmu6
# - 权限问题: chmod +x UnrealEditor
# - 存储不足: df -h
```

### 问题 4: 视频文件过大
```bash
# 降低质量（更快，更小）
ffmpeg ... -crf 28 ...

# 降低帧率
ffmpeg ... -framerate 15 ...

# 降低分辨率
ffmpeg ... -video_size 1280x720 ...
```

---

## 📊 性能参考

**RTX 5090 服务器预期性能**：
```
视频录制: 1920x1080 @ 30fps
编码速度: ~2x 实时（30秒视频 ~15秒编码）
文件大小: ~50MB/分钟 (CRF 23)
GPU 占用: ~30-50%（录制时）
```

---

## 🚀 快速开始

```bash
# 1. 初始化服务器（仅首次）
./tools/server-init.sh

# 2. 录制测试视频
./tools/quick-record.sh 10 test

# 3. 传输到本地查看
scp user@server:/tmp/test.mp4 ./
```

---

## 📝 备注

- 所有脚本假设 UE5 安装在 `/data/UE5`
- 项目路径: `/data/project/FirstGame`
- 可通过环境变量覆盖: `WORK_DIR=/custom/path ./quick-record.sh`
- 视频默认保存到 `/tmp`，可手动移动
