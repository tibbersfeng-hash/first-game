#!/bin/bash
# 快速录制游戏视频（简化版）
# 用法: quick-record.sh [30|60|120] [output_name]
# 示例: quick-record.sh 60 combat-test

set -e

DURATION=${1:-30}
NAME=${2:-"gameplay-$(date +%Y%m%d-%H%M%S)"}
OUTPUT="/tmp/${NAME}.mp4"

WORK_DIR="${WORK_DIR:-/data}"
UE5="$WORK_DIR/UE5/Engine/Binaries/Linux/UnrealEditor"
PROJECT="$WORK_DIR/project/FirstGame/FirstGame.uproject"

echo "🎬 录制游戏视频 | ${DURATION}s → ${OUTPUT}"

# 清理旧进程
pkill -f "Xvfb :99" 2>/dev/null || true
sleep 1

# 启动虚拟显示
DISPLAY=:99 Xvfb :99 -screen 0 1920x1080x24 +extension GLX &
sleep 2

# 后台录制
DISPLAY=:99 ffmpeg -f x11grab -video_size 1920x1080 -framerate 30 -i :99 \
    -c:v libx264 -preset fast -crf 23 -t $DURATION "$OUTPUT" 2>/dev/null &

# 运行游戏
DISPLAY=:99 vglrun "$UE5" "$PROJECT" \
    -game -windowed -resX=1920 -resY=1080 \
    -nosplash -nosteam -nosound 2>/dev/null &
UE5_PID=$!

# 等待
echo "⏳ 录制中... (${DURATION}秒)"
sleep $DURATION

# 清理
kill $UE5_PID 2>/dev/null || true
pkill -f "ffmpeg.*x11grab" 2>/dev/null || true
pkill -f "Xvfb :99" 2>/dev/null || true

# 验证
if [ -f "$OUTPUT" ]; then
    SIZE=$(du -h "$OUTPUT" | cut -f1)
    echo "✅ 录制完成: ${OUTPUT} (${SIZE})"
    echo ""
    echo "📤 传输命令:"
    echo "   scp $(whoami)@$(hostname -I | awk '{print $1}'):${OUTPUT} ./"
else
    echo "❌ 录制失败"
    exit 1
fi
