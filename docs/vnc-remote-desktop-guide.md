# VNC 远程桌面连接指南

## 当前状态

✅ **VNC 服务已启动**
- 服务器：`connect.nmb2.seetacloud.com:42491` (gpu)
- VNC 端口：5901 (display :1)
- 分辨率：1920x1080
- 桌面环境：XFCE4
- 状态：运行中

## 连接方法

### 方法 1：SSH 端口转发 + VNC 客户端（推荐）

**步骤 1：建立 SSH 隧道**

```bash
# 在本地终端执行
ssh -L 5901:localhost:5901 root@connect.nmb2.seetacloud.com -p 42491
```

这会创建一个 SSH 隧道，将本地的 5901 端口转发到远程服务器的 5901 端口。

**步骤 2：使用 VNC 客户端连接**

使用任意 VNC 客户端连接到：
- 地址：`localhost:5901` 或 `127.0.0.1:5901`
- 密码：需要查看或设置 VNC 密码（见下方）

### 方法 2：AutoDL 控制台（如果有 Web VNC）

如果 AutoDL 平台提供 Web VNC 功能：
1. 登录 AutoDL 控制台
2. 找到实例的 "远程连接" 或 "VNC" 选项
3. 直接通过浏览器访问

## VNC 密码管理

### 查看当前密码

```bash
ssh gpu "cat ~/.vnc/passwd"
```

### 修改 VNC 密码

```bash
ssh gpu "vncpasswd"
```

按提示输入新密码（6-8 位）。

## 启动/停止 VNC 服务

### 启动 VNC

```bash
ssh gpu "vncserver :1 -geometry 1920x1080 -depth 24"
```

### 停止 VNC

```bash
ssh gpu "vncserver -kill :1"
```

### 查看运行状态

```bash
ssh gpu "vncserver -list"
```

## 在 VNC 中启动 UE5 Editor

连接 VNC 后，打开终端执行：

```bash
cd /root/autodl-tmp/ue5/Engine/Binaries/Linux
./UnrealEditor /root/autodl-tmp/project/first-game/src_ue5/FirstGame.uproject
```

## 故障排除

### 问题 1：VNC 连接被拒绝

**原因**：SSH 隧道未建立或 VNC 服务未运行

**解决**：
```bash
# 检查 VNC 服务
ssh gpu "vncserver -list"

# 如果未运行，启动它
ssh gpu "vncserver :1 -geometry 1920x1080 -depth 24"

# 重新建立 SSH 隧道
ssh -L 5901:localhost:5901 root@connect.nmb2.seetacloud.com -p 42491
```

### 问题 2：VNC 显示黑屏

**原因**：桌面环境未正确启动

**解决**：
```bash
# 检查 xstartup 配置
ssh gpu "cat ~/.vnc/xstartup"

# 应该包含：exec startxfce4
# 如果不对，重新配置
ssh gpu "echo 'exec startxfce4' > ~/.vnc/xstartup"
ssh gpu "chmod +x ~/.vnc/xstartup"

# 重启 VNC
ssh gpu "vncserver -kill :1"
ssh gpu "vncserver :1 -geometry 1920x1080 -depth 24"
```

### 问题 3：UE5 Editor 无法启动

**原因**：缺少显示环境变量

**解决**：在 VNC 桌面中打开终端，确保设置了 `DISPLAY` 环境变量：
```bash
export DISPLAY=:1
./UnrealEditor /path/to/project.uproject
```

## 性能优化建议

### 降低 VNC 画质（提高流畅度）

启动 VNC 时添加参数：
```bash
vncserver :1 -geometry 1920x1080 -depth 16 -quality 5
```

### 使用硬件编码（如果支持）

```bash
vncserver :1 -geometry 1920x1080 -depth 24 -codecID 1
```

## 下一步

1. ✅ VNC 服务已启动
2. ⏳ 建立 SSH 隧道
3. ⏳ 连接 VNC 客户端
4. ⏳ 在 VNC 中启动 UE5 Editor
5. ⏳ 执行 3D 资产导入脚本

## 推荐的 VNC 客户端

- **macOS**: 系统自带 "屏幕共享"，或使用 RealVNC Viewer
- **Windows**: RealVNC Viewer, TightVNC, TigerVNC Viewer
- **Linux**: Remmina, TigerVNC Viewer, Vinagre

## 替代方案

如果 VNC 连接不稳定，可以考虑：

1. **Parsec**（更低延迟，需要安装）
2. **NoMachine**（高性能远程桌面）
3. **X11 转发**（仅转发单个窗口）：
   ```bash
   ssh -X root@connect.nmb2.seetacloud.com -p 42491
   ./UnrealEditor /path/to/project.uproject
   ```
