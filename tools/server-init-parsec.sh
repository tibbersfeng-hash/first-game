#!/bin/bash
# 游戏服务器初始化脚本 — Parsec 远程桌面版
# 目标：SSH 操作 + Parsec 远程桌面（低延迟 GPU 加速）

set -e

echo "=== 游戏服务器初始化（Parsec 版）==="

# ─── 1. 系统更新 + 基础工具 ───────────────────────────────────────
echo "[1/9] 安装基础工具..."
sudo apt update && sudo apt upgrade -y
sudo apt install -y \
    build-essential \
    cmake \
    git \
    git-lfs \
    rsync \
    wget \
    curl \
    unzip \
    python3 \
    python3-pip \
    vim \
    htop \
    tmux \
    mesa-utils

# ─── 2. 检查 GPU 驱动 ─────────────────────────────────────────────
echo "[2/9] 检查 NVIDIA 驱动..."
if ! command -v nvidia-smi &> /dev/null; then
    echo "❌ NVIDIA 驱动未安装"
    echo "请先安装 CUDA 12.8 + NVIDIA 驱动"
    exit 1
fi

nvidia-smi
echo "✓ GPU 驱动正常"

# ─── 3. 安装桌面环境（Parsec 需要） ───────────────────────────────
echo "[3/9] 安装轻量桌面环境..."
sudo apt install -y xfce4 xfce4-goodies

# 设置默认桌面
echo "exec startxfce4" > ~/.xsession
chmod +x ~/.xsession

# ─── 4. 安装虚拟显示（headless 服务器需要） ───────────────────────
echo "[4/9] 安装虚拟显示..."
sudo apt install -y \
    xvfb \
    x11-xserver-utils \
    x11-utils

# 安装 dummy driver（虚拟显示器）
sudo apt install -y xserver-xorg-video-dummy

# 创建 Xorg 配置（虚拟显示器）
sudo tee /etc/X11/xorg.conf > /dev/null << 'XORG_EOF'
Section "Device"
    Identifier "Configured Video Device"
    Driver "dummy"
    VideoRam 256000
EndSection

Section "Monitor"
    Identifier "Configured Monitor"
    HorizSync 31.5-48.0
    VertRefresh 50.0-70.0
    Modeline "1920x1080" 148.50 1920 2008 2052 2200 1080 1084 1089 1125 +hsync +vsync
EndSection

Section "Screen"
    Identifier "Default Screen"
    Monitor "Configured Monitor"
    Device "Configured Video Device"
    DefaultDepth 24
    SubSection "Display"
        Depth 24
        Modes "1920x1080"
    EndSubSection
EndSection
XORG_EOF

echo "✓ 虚拟显示配置完成"

# ─── 5. 安装 Parsec ───────────────────────────────────────────────
echo "[5/9] 安装 Parsec..."
cd /tmp

# 下载 Parsec
wget -q https://builds.parsec.app/package/parsec-linux.deb
sudo dpkg -i parsec-linux.deb || sudo apt-get install -f -y
rm parsec-linux.deb

# 启动 Parsec（headless 模式）
echo "启动 Parsec..."
sudo systemctl enable parsecd
sudo systemctl start parsecd

echo "✓ Parsec 已安装并启动"

# ─── 6. 安装 ffmpeg + 录制工具 ────────────────────────────────────
echo "[6/9] 安装视频录制工具..."
sudo apt install -y ffmpeg

# ─── 7. 检查存储 ───────────────────────────────────────────────────
echo "[7/9] 检查存储..."
df -h

if [ ! -d "/data" ]; then
    echo "⚠️  /data 目录不存在"
    echo "建议挂载数据盘到 /data（至少 300GB）"
    df -h /
    read -p "是否继续？(y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
    WORK_DIR="/home/$USER"
else
    WORK_DIR="/data"
fi

# ─── 8. 创建项目目录 ───────────────────────────────────────────────
echo "[8/9] 创建项目目录..."
sudo mkdir -p $WORK_DIR/project
sudo mkdir -p $WORK_DIR/UE5
sudo chown -R $USER:$USER $WORK_DIR

echo "工作目录: $WORK_DIR"

# ─── 9. UE5 + 项目准备 ─────────────────────────────────────────────
echo "[9/9] UE5 + 项目准备..."
echo ""
echo "⚠️  需要手动完成以下步骤："
echo ""
echo "1️⃣  安装 UE5.7.4:"
echo "   方式 A: 从旧服务器迁移"
echo "     rsync -avz root@old-server:/root/autodl-tmp/UE5/ $WORK_DIR/UE5/"
echo ""
echo "   方式 B: 从 Epic Games 下载"
echo "     访问 https://www.unrealengine.com/download"
echo ""
echo "2️⃣  克隆项目:"
echo "   git clone git@github.com:tibbersfeng-hash/first-game.git $WORK_DIR/project/FirstGame"
echo ""
read -p "按回车继续..."

# 验证
if [ ! -f "$WORK_DIR/UE5/Engine/Binaries/Linux/UnrealEditor" ]; then
    echo "⚠️  UE5 未找到，请先安装"
fi

# ─── 创建辅助脚本 ──────────────────────────────────────────────────
echo "创建辅助脚本..."

# Parsec 状态检查
cat > $WORK_DIR/parsec-status.sh << 'EOF'
#!/bin/bash
echo "=== Parsec 状态 ==="
sudo systemctl status parsecd --no-pager | head -15
echo ""
echo "Parsec 配对 ID:"
cat /etc/parsec/parsec.conf 2>/dev/null | grep pair || echo "未配置"
echo ""
echo "连接地址:"
hostname -I | awk '{print $1}'
EOF
chmod +x $WORK_DIR/parsec-status.sh

# 虚拟显示管理
cat > $WORK_DIR/display-start.sh << 'EOF'
#!/bin/bash
echo "启动虚拟显示..."
sudo systemctl start xorg
sleep 2
export DISPLAY=:0
echo "✓ DISPLAY=$DISPLAY"
echo "现在可以通过 Parsec 连接"
EOF
chmod +x $WORK_DIR/display-start.sh

cat > $WORK_DIR/display-stop.sh << 'EOF'
#!/bin/bash
echo "停止虚拟显示..."
sudo systemctl stop xorg
echo "✓ 虚拟显示已停止"
EOF
chmod +x $WORK_DIR/display-stop.sh

# ─── 完成 ──────────────────────────────────────────────────────────
echo ""
echo "=========================================="
echo "✅ 初始化完成！"
echo "=========================================="
echo ""
echo "📍 下一步操作："
echo ""
echo "1️⃣  获取 Parsec 配对码"
echo "   $WORK_DIR/parsec-status.sh"
echo ""
echo "2️⃣  在本地电脑安装 Parsec"
echo "   下载: https://parsec.app/downloads"
echo ""
echo "3️⃣  添加远程机器"
echo "   - 打开 Parsec → 添加机器"
echo "   - 输入服务器 IP 和配对码"
echo ""
echo "4️⃣  连接远程桌面"
echo "   - 点击连接"
echo "   - 应该看到 XFCE 桌面"
echo ""
echo "5️⃣  启动 UE5 Editor"
echo "   - 在远程桌面中打开终端"
echo "   - cd $WORK_DIR/project/FirstGame"
echo "   - $WORK_DIR/UE5/Engine/Binaries/Linux/UnrealEditor FirstGame.uproject"
echo ""
echo "=========================================="
echo ""
echo "辅助脚本："
echo "  $WORK_DIR/parsec-status.sh    # 查看 Parsec 状态"
echo "  $WORK_DIR/display-start.sh    # 启动虚拟显示"
echo "  $WORK_DIR/display-stop.sh     # 停止虚拟显示"
echo ""
