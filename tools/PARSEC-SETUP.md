# Parsec 远程桌面设置指南

> RTX 5090 服务器 + Parsec = 低延迟 UE5 开发环境

---

## 🎯 方案概述

```
你（本地电脑）                    GPU 服务器（RTX 5090）
┌──────────────┐                 ┌──────────────────────┐
│ Parsec 客户端 │◄──────────────►│ Parsec 服务端        │
│              │   互联网       │                      │
│ 看到远程桌面 │   延迟 <10ms   │ XFCE 桌面            │
│ 操作 UE5     │   60fps        │ UE5 Editor           │
└──────────────┘                 └──────────────────────┘
                                         ↑
                                  VirtualGL + GPU 加速
```

**优势**：
- ✅ 延迟极低（<10ms）
- ✅ 60fps 流畅体验
- ✅ 支持 4K 分辨率
- ✅ 利用 RTX 5090 硬件编码
- ✅ 可同时用于 AI 开发（SSH）和人工查看（Parsec）

---

## 📋 服务器端设置（一次性）

### 1. 运行初始化脚本

```bash
# SSH 登录服务器
ssh root@your-server

# 运行初始化脚本
cd first-game/tools
./server-init-parsec.sh
```

**脚本会自动安装**：
- XFCE4 桌面环境（轻量）
- VirtualGL（GPU 加速）
- Parsec 服务端
- Xvfb + dummy driver（虚拟显示器）
- ffmpeg（视频录制备用）

### 2. 获取 Parsec 配对码

```bash
# 查看 Parsec 状态和配对信息
/data/parsec-status.sh
```

输出示例：
```
=== Parsec 状态 ===
● parsecd.service - Parsec Daemon
   Active: active (running)
   
Parsec 配对 ID:
pair=xxxxxxxxxxxxxxxx

连接地址:
192.168.1.100
```

### 3. 安装 UE5 和项目

```bash
# 从旧服务器迁移 UE5
rsync -avz root@old-server:/root/autodl-tmp/UE5/ /data/UE5/

# 克隆项目
git clone git@github.com:tibbersfeng-hash/first-game.git /data/project/FirstGame
```

---

## 💻 客户端设置（你的电脑）

### 1. 安装 Parsec

```
下载地址: https://parsec.app/downloads

支持平台:
- Windows 10/11
- macOS 10.15+
- Linux (Ubuntu 20.04+)
```

### 2. 登录账号

```
1. 打开 Parsec
2. 登录你的账号（没有就注册一个，免费）
3. 进入主界面
```

### 3. 添加远程服务器

```
方式 A: 同一局域网
1. 点击 "Add Machine"
2. 选择 "Manual Pairing"
3. 输入服务器 IP: 192.168.1.100
4. 输入配对码: pair=xxxxxxxxxxxxxxxx
5. 点击 "Add"

方式 B: 跨网络（公网）
1. 服务器需要开放端口:
   - UDP 9000-9100
   - TCP 8080
2. 使用服务器的公网 IP
3. 其他步骤同上
```

### 4. 连接远程桌面

```
1. 在 Parsec 主机列表找到你的服务器
2. 点击 "Connect"
3. 等待连接建立
4. 应该看到 XFCE 桌面环境
```

---

## 🎮 使用 UE5 Editor

### 在 Parsec 远程桌面中启动 UE5

```bash
# 在远程桌面的终端中
cd /data/project/FirstGame
/data/UE5/Engine/Binaries/Linux/UnrealEditor FirstGame.uproject
```

**预期效果**：
- UE5 Editor 窗口出现在远程桌面
- 可以用鼠标直接操作
- 实时预览游戏画面
- 60fps 流畅体验

### 优化设置

**Parsec 客户端设置**（推荐）：
```
Settings → Client → 
  Resolution: 1920x1080 (或 4K 如果显示器支持)
  Frame Rate: 60 FPS
  Bandwidth Limit: 50 Mbps (或更高)
  Decoder: Hardware (利用本地 GPU 解码)
```

**UE5 Editor 设置**（远程桌面内）：
```
Editor Preferences → Performance →
  Real-time Capture Frame Rate: 60
  Use GPU Skin Cache: ✓
  
Level Editor → Viewport →
  Realtime: ✓
  Buffer Visualization: None
```

---

## 🔧 故障排查

### 问题 1: Parsec 无法连接

```bash
# 检查 Parsec 服务状态
sudo systemctl status parsecd

# 重启服务
sudo systemctl restart parsecd

# 查看日志
sudo journalctl -u parsecd -n 50
```

**常见原因**：
- 防火墙阻止端口（UDP 9000-9100）
- 网络不稳定
- 配对码过期（重新获取）

### 问题 2: 连接后黑屏

```bash
# 检查虚拟显示是否运行
ps aux | grep Xorg

# 启动虚拟显示
/data/display-start.sh

# 检查 DISPLAY 环境变量
echo $DISPLAY  # 应该是 :0
```

### 问题 3: 画面卡顿 / 延迟高

**客户端优化**：
```
Parsec Settings →
  Bandwidth: 提高到 50-100 Mbps
  Resolution: 降低到 1080p
  Frame Rate: 降低到 30 FPS
```

**服务器优化**：
```bash
# 检查 GPU 占用
nvidia-smi

# 检查网络延迟
ping your-client-ip

# 关闭不必要的后台程序
htop
```

### 问题 4: UE5 启动失败（在远程桌面中）

```bash
# 检查日志
tail -f /data/project/FirstGame/Saved/Logs/FirstGame.log

# 常见问题：
# 1. 缺少库: sudo apt install libxi6 libxmu6 libxtst6
# 2. 权限问题: chmod +x UnrealEditor
# 3. 虚拟显示未启动: /data/display-start.sh
```

---

## 🎬 视频录制（可选）

即使有 Parsec，有时也需要录制视频分享给别人：

```bash
# 使用 quick-record.sh（需要 Xvfb 模式）
/data/quick-record.sh 60 combat-demo

# 或者在 Parsec 远程桌面中使用 OBS
# 1. 在远程桌面安装 OBS
# 2. 录制整个桌面
# 3. 保存到 /data/videos/
```

---

## 📊 性能参考

**RTX 5090 + Parsec 预期性能**：
```
分辨率: 1920x1080 @ 60fps
延迟: < 10ms（局域网）, 20-50ms（公网）
画质: 几乎无损（硬件编码 H.265）
GPU 占用: ~10-20%（Parsec 编码）
带宽: 20-50 Mbps
```

**实际体验**：
- UE5 Editor 操作流畅
- 实时预览无卡顿
- 蓝图编辑响应迅速
- 游戏测试 60fps 稳定

---

## 🚀 快速开始

```bash
# === 服务器端（SSH）===
ssh root@your-server
cd first-game/tools
./server-init-parsec.sh
/data/parsec-status.sh  # 获取配对码

# === 客户端（你的电脑）===
# 1. 安装 Parsec: https://parsec.app/downloads
# 2. 登录账号
# 3. Add Machine → 输入 IP + 配对码
# 4. Connect → 看到远程桌面
# 5. 在远程桌面中启动 UE5
```

---

## 📝 备注

- Parsec 个人版免费，足够使用
- 服务器需要公网 IP 或端口转发才能跨网络连接
- 同一局域网内可以直接用内网 IP
- 可以同时用 SSH（AI 开发）+ Parsec（人工查看）
- 建议服务器配置静态 IP，避免 IP 变化

---

## 🔗 相关资源

- Parsec 官方文档: https://parsec.app/docs
- Parsec Linux 安装: https://parsec.app/knowledge-base/installing-parsec-linux
- UE5 Linux 部署指南: https://docs.unrealengine.com/5.7/en-US/deploying-unreal-engine-on-linux/
