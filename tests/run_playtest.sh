#!/bin/bash
# 格斗萌主 — 完整功能测试 (Xvfb + Godot 内置截图)
set -euo pipefail

PROJECT_DIR="/home/claude/.frontend/first-game"
GODOT="/home/claude/.local/bin/godot"
OUTPUT_DIR="/tmp/first-game-playtest"
DISPLAY_NUM=95

rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🎮 格斗萌主 — 完整功能测试 (Xvfb + Godot 截图)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# 1. Xvfb
echo "🖥️  启动 Xvfb :$DISPLAY_NUM"
Xvfb ":$DISPLAY_NUM" -screen 0 1280x720x24 -ac +extension GLX +render -noreset &
XVFB_PID=$!
sleep 2
kill -0 $XVFB_PID 2>/dev/null || { echo "❌ Xvfb 失败"; exit 1; }
echo "   ✅ PID=$XVFB_PID"

# 2. Godot (带渲染)
echo ""
echo "🎮 运行 Godot..."
echo "───────────────────────────────────────────────"

cd "$PROJECT_DIR"
SCREENSHOT_DIR="$OUTPUT_DIR" DISPLAY=":$DISPLAY_NUM" \
    timeout 20 "$GODOT" --rendering-driver opengl3 \
    res://tests/playtest_recorder.tscn 2>&1 | tee "$OUTPUT_DIR/game_log.txt" || true

# 3. 停止 Xvfb
kill $XVFB_PID 2>/dev/null || true

# 4. 汇总
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 汇总"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

PNG_COUNT=$(ls -1 "$OUTPUT_DIR"/shot_*.png 2>/dev/null | wc -l)
echo ""
echo "📸 截图: $PNG_COUNT 张 → $OUTPUT_DIR/"

# 列出截图
ls -la "$OUTPUT_DIR"/shot_*.png 2>/dev/null | head -10

# 生成 GIF
if [ "$PNG_COUNT" -gt 2 ]; then
    echo ""
    echo "🎬 生成 GIF..."
    ffmpeg -y -framerate 6 -i "$OUTPUT_DIR/shot_%04d.png" \
        -vf "scale=640:360:flags=neighbor" \
        "$OUTPUT_DIR/combat.gif" -loglevel error 2>&1 || true
    echo "   → $OUTPUT_DIR/combat.gif"
fi

# 关键事件
echo ""
echo "📋 关键事件:"
grep -E "🎯|☠️|🔗|💥|⚔️|🏃|✅|❌|🎉|⚠️" "$OUTPUT_DIR/game_log.txt" 2>/dev/null || echo "   (无)"

# 判定
echo ""
if grep -q "🎉 通过" "$OUTPUT_DIR/game_log.txt" 2>/dev/null; then
    echo "🎉 测试通过!"
    echo ""
    echo "📁 $OUTPUT_DIR/"
    echo "   shot_*.png   — Godot 内置截图 (真实渲染)"
    echo "   combat.gif   — 战斗动画"
    echo "   game_log.txt — 完整日志"
    exit 0
else
    echo "⚠️ 未通过 — 查看日志: $OUTPUT_DIR/game_log.txt"
    exit 1
fi
