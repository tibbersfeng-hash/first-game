#!/bin/bash
# 游戏服务器初始化脚本 — AI 自主开发 + 视频录制
# 目标：纯 SSH 操作，可录制游戏运行视频

set -e

echo "=== 游戏服务器初始化 ==="

# ─── 1. 系统更新 + 基础工具 ───────────────────────────────────────
echo "[1/8] 安装基础工具..."
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
    tmux

# ─── 2. 检查 GPU 驱动 ─────────────────────────────────────────────
echo "[2/8] 检查 NVIDIA 驱动..."
if ! command -v nvidia-smi &> /dev/null; then
    echo "⚠️  NVIDIA 驱动未安装，需要手动安装 CUDA 12.8 + 驱动"
    echo "参考: https://docs.nvidia.com/cuda/cuda-installation-guide-linux/"
    exit 1
fi

nvidia-smi
echo "✓ GPU 驱动正常"

# ─── 3. 安装虚拟显示工具（GPU 加速） ──────────────────────────────
echo "[3/8] 安装虚拟显示工具..."
sudo apt install -y \
    xvfb \
    x11-xserver-utils \
    x11-utils \
    mesa-utils

# 安装 VirtualGL（GPU 加速渲染）
echo "安装 VirtualGL..."
cd /tmp
wget -q https://github.com/VirtualGL/virtualgl/releases/download/3.1.1/virtualgl_3.1.1_amd64.deb
sudo dpkg -i virtualgl_3.1.1_amd64.deb || sudo apt-get install -f -y
rm virtualgl_3.1.1_amd64.deb

# 配置 VirtualGL
sudo /opt/VirtualGL/bin/vglserver_config +glx +confirm +f

# ─── 4. 安装 ffmpeg（视频录制） ───────────────────────────────────
echo "[4/8] 安装 ffmpeg..."
sudo apt install -y ffmpeg

# ─── 5. 检查存储 ───────────────────────────────────────────────────
echo "[5/8] 检查存储..."
df -h

# 检查 /data 目录（数据盘）
if [ ! -d "/data" ]; then
    echo "⚠️  /data 目录不存在，建议挂载数据盘"
    echo "当前磁盘使用:"
    df -h /
    read -p "是否继续？(y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# ─── 6. 创建项目目录 ───────────────────────────────────────────────
echo "[6/8] 创建项目目录..."
WORK_DIR="${WORK_DIR:-/data}"
sudo mkdir -p $WORK_DIR/project
sudo mkdir -p $WORK_DIR/UE5
sudo chown -R $USER:$USER $WORK_DIR

echo "工作目录: $WORK_DIR"

# ─── 7. 安装 UE5.7.4（命令行） ─────────────────────────────────────
echo "[7/8] UE5.7.4 安装..."
echo "⚠️  UE5 需要从 Epic Games 下载或从旧服务器迁移"
echo ""
echo "选项 A: 从旧服务器 rsync 迁移"
echo "  rsync -avz root@old-server:/root/autodl-tmp/UE5/ $WORK_DIR/UE5/"
echo ""
echo "选项 B: 从 Epic Games 下载（需要账号）"
echo "  访问 https://www.unrealengine.com/download"
echo ""
read -p "UE5 是否已安装到 $WORK_DIR/UE5? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "请先安装 UE5，然后重新运行此脚本"
    exit 1
fi

# 验证 UE5
if [ ! -f "$WORK_DIR/UE5/Engine/Binaries/Linux/UnrealEditor" ]; then
    echo "❌ UE5 未找到: $WORK_DIR/UE5/Engine/Binaries/Linux/UnrealEditor"
    exit 1
fi
echo "✓ UE5 已安装"

# ─── 8. 项目迁移 ───────────────────────────────────────────────────
echo "[8/8] 项目迁移..."
PROJECT_DIR="$WORK_DIR/project/FirstGame"

if [ ! -d "$PROJECT_DIR" ]; then
    echo "⚠️  项目目录不存在: $PROJECT_DIR"
    echo ""
    echo "从 GitHub 克隆:"
    echo "  git clone git@github.com:tibbersfeng-hash/first-game.git $PROJECT_DIR"
    echo ""
    echo "或从旧服务器迁移:"
    echo "  rsync -avz root@old-server:/root/project/FirstGame/ $PROJECT_DIR/"
    echo ""
    read -p "项目是否已准备好? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo "✓ 项目目录: $PROJECT_DIR"

# ─── 创建视频录制脚本 ──────────────────────────────────────────────
echo "创建视频录制工具..."
cat > $WORK_DIR/record-gameplay.sh << 'RECORD_EOF'
#!/bin/bash
# 录制游戏运行视频
# 用法: ./record-gameplay.sh [时长秒] [输出文件]

set -e

DURATION=${1:-30}
OUTPUT=${2:-"/tmp/gameplay-$(date +%Y%m%d-%H%M%S).mp4"}
WORK_DIR="${WORK_DIR:-/data}"
PROJECT_DIR="$WORK_DIR/project/FirstGame"
UE5_DIR="$WORK_DIR/UE5"

echo "=== 录制游戏视频 ==="
echo "时长: ${DURATION}秒"
echo "输出: $OUTPUT"

# 启动虚拟显示（GPU 加速）
echo "启动虚拟显示..."
export DISPLAY=:99
Xvfb :99 -screen 0 1920x1080x24 +extension GLX &
XVFB_PID=$!
sleep 2

# 启动 ffmpeg 录制
echo "启动录制..."
ffmpeg -f x11grab -video_size 1920x1080 -framerate 30 -i :99 \
    -c:v libx264 -preset fast -crf 23 \
    -t $DURATION \
    "$OUTPUT" &
FFMPEG_PID=$!

# 运行 UE5 游戏
echo "启动 UE5 游戏..."
vglrun $UE5_DIR/Engine/Binaries/Linux/UnrealEditor \
    $PROJECT_DIR/FirstGame.uproject \
    -game -windowed -resX=1920 -resY=1080 \
    -nosplash -nosteam -nosound \
    -windowed -ForceRes &
UE5_PID=$!

# 等待录制完成
echo "录制中... (${DURATION}秒)"
sleep $DURATION

# 停止 UE5
echo "停止 UE5..."
kill $UE5_PID 2>/dev/null || true
wait $UE5_PID 2>/dev/null || true

# 停止 ffmpeg
echo "停止录制..."
kill $FFMPEG_PID 2>/dev/null || true
wait $FFMPEG_PID 2>/dev/null || true

# 停止虚拟显示
echo "清理虚拟显示..."
kill $XVFB_PID 2>/dev/null || true

echo "✓ 视频已保存: $OUTPUT"
echo "传输到本地: scp user@server:$OUTPUT ./gameplay.mp4"
RECORD_EOF

chmod +x $WORK_DIR/record-gameplay.sh

# ─── 完成 ──────────────────────────────────────────────────────────
echo ""
echo "=== 初始化完成 ==="
echo ""
echo "工作目录: $WORK_DIR"
echo "UE5 路径: $WORK_DIR/UE5"
echo "项目路径: $PROJECT_DIR"
echo "录制脚本: $WORK_DIR/record-gameplay.sh"
echo ""
echo "使用示例:"
echo "  # 录制 30 秒游戏视频"
echo "  $WORK_DIR/record-gameplay.sh 30"
echo ""
echo "  # 录制 60 秒并指定输出"
echo "  $WORK_DIR/record-gameplay.sh 60 /tmp/my-video.mp4"
echo ""
echo "  # 传输视频到本地"
echo "  scp user@server:/tmp/gameplay-*.mp4 ./"
echo ""
