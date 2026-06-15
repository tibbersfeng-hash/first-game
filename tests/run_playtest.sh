#!/bin/bash
# 格斗萌主 — 完整功能测试 (Xvfb + Godot + ffmpeg 录像)
set -euo pipefail

PROJECT_DIR="/home/claude/.frontend/first-game"
GODOT="/home/claude/.local/bin/godot"
OUTPUT_DIR="/tmp/first-game-playtest"
DISPLAY_NUM=89

rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🎮 格斗萌主 — 完整功能测试 + 录像"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# 1. Xvfb
echo "🖥️  启动 Xvfb :$DISPLAY_NUM"
Xvfb ":$DISPLAY_NUM" -screen 0 1280x720x24 -ac +extension GLX +render -noreset &
XVFB_PID=$!
sleep 2

# 2. Godot（带测试脚本）
echo "🎮 启动 Godot (自动战斗)"
DISPLAY=":$DISPLAY_NUM" "$GODOT" --rendering-driver opengl3 \
    res://tests/auto_combat.tscn 2>/dev/null &
GODOT_PID=$!

# 3. ffmpeg 截图循环（每0.4秒截一张，最多40张）
echo "📸 开始录像 (每0.4秒截图)"
FRAME=0
while kill -0 $GODOT_PID 2>/dev/null && [ $FRAME -lt 40 ]; do
    FRAME=$((FRAME + 1))
    DISPLAY=":$DISPLAY_NUM" ffmpeg -y -f x11grab -video_size 1280x720 \
        -i ":$DISPLAY_NUM" -frames:v 1 -update 1 \
        "$OUTPUT_DIR/frame_$(printf '%04d' $FRAME).png" \
        -loglevel error 2>/dev/null || true
    sleep 0.4
done

echo "   录像完成: $FRAME 帧"

# 4. 等待 Godot 退出
wait $GODOT_PID 2>/dev/null || true
kill $XVFB_PID 2>/dev/null

# 5. 生成 GIF
FRAME_COUNT=$(ls -1 "$OUTPUT_DIR"/frame_*.png 2>/dev/null | wc -l)
echo ""
echo "🎬 生成 GIF ($FRAME_COUNT 帧)..."

if [ "$FRAME_COUNT" -gt 2 ]; then
    ffmpeg -y -framerate 5 -i "$OUTPUT_DIR/frame_%04d.png" \
        -vf "scale=640:360:flags=neighbor,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" \
        "$OUTPUT_DIR/combat.gif" -loglevel error 2>/dev/null || \
    ffmpeg -y -framerate 5 -i "$OUTPUT_DIR/frame_%04d.png" \
        -vf "scale=640:360" "$OUTPUT_DIR/combat.gif" -loglevel error 2>/dev/null || true
fi

# 6. 汇总
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 结果"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📁 输出: $OUTPUT_DIR/"
echo "   frame_XXXX.png — 游戏截图序列"
if [ -f "$OUTPUT_DIR/combat.gif" ]; then
    SIZE=$(du -h "$OUTPUT_DIR/combat.gif" | cut -f1)
    echo "   combat.gif     — 战斗录像 ($SIZE)"
fi
echo ""
echo "💡 查看: xdg-open $OUTPUT_DIR/frame_0001.png"
