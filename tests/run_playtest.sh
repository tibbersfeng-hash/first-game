#!/bin/bash
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# 格斗萌主 — 完整功能测试脚本
# 使用 Xvfb 虚拟显示 + Godot + 自动截图
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

set -euo pipefail

PROJECT_DIR="/home/claude/.frontend/first-game"
GODOT="/home/claude/.local/bin/godot"
OUTPUT_DIR="/tmp/first-game-playtest"
DISPLAY_NUM=99
SCREENSHOT_INTERVAL=0.5  # 秒

# 清理旧输出
rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🎮 格斗萌主 — 完整功能测试 (Xvfb + Godot)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# ─── Step 1: 启动 Xvfb 虚拟显示 ───
echo "🖥️  启动虚拟显示 :$DISPLAY_NUM (1280x720x24)"
Xvfb ":$DISPLAY_NUM" -screen 0 1280x720x24 -ac &
XVFB_PID=$!
sleep 1

# 验证 Xvfb 运行
if ! kill -0 $XVFB_PID 2>/dev/null; then
    echo "❌ Xvfb 启动失败"
    exit 1
fi
echo "   ✅ Xvfb PID=$XVFB_PID"

# ─── Step 2: 截图循环 (后台) ───
echo "📸 启动自动截图 (每${SCREENSHOT_INTERVAL}秒)"
FRAME_COUNT=0
(
    while true; do
        FRAME_COUNT=$((FRAME_COUNT + 1))
        ffmpeg -y -f x11grab -video_size 1280x720 -i ":$DISPLAY_NUM" \
            -frames:v 1 "$OUTPUT_DIR/frame_$(printf '%04d' $FRAME_COUNT).png" \
            -loglevel error 2>/dev/null || true
        sleep "$SCREENSHOT_INTERVAL"
    done
) &
SCREENSHOT_PID=$!
echo "   ✅ 截图进程 PID=$SCREENSHOT_PID"

# ─── Step 3: 运行 Godot (带虚拟显示) ───
echo ""
echo "🎮 运行 Godot 战斗场景..."
echo "───────────────────────────────────────────────────"

cd "$PROJECT_DIR"
RECORD_GAME=1 DISPLAY=":$DISPLAY_NUM" timeout 20 "$GODOT" --rendering-driver opengl3 \
    res://tests/playtest_recorder.tscn 2>&1 | tee "$OUTPUT_DIR/game_log.txt" || true

GODOT_EXIT=$?
echo ""
echo "   Godot 退出码: $GODOT_EXIT"

# ─── Step 4: 停止后台进程 ───
kill $SCREENSHOT_PID 2>/dev/null || true
kill $XVFB_PID 2>/dev/null || true
wait 2>/dev/null || true

# ─── Step 5: 汇总结果 ───
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 测试结果汇总"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# 截图统计
SCREENSHOT_COUNT=$(ls -1 "$OUTPUT_DIR"/frame_*.png 2>/dev/null | wc -l)
echo ""
echo "📸 截图: $SCREENSHOT_COUNT 张 → $OUTPUT_DIR/"

# 日志分析
echo ""
echo "📋 关键事件:"
grep -E "🎯|☠️|🔗|💥|⚔️|🏃|✅|❌|🎉|⚠️" "$OUTPUT_DIR/game_log.txt" 2>/dev/null || echo "   (无事件)"

# 最终判定
echo ""
if grep -q "🎉 完整验证通过" "$OUTPUT_DIR/game_log.txt" 2>/dev/null; then
    echo "🎉 测试通过! 游戏在虚拟显示环境下完整运行"
    echo ""
    echo "📁 输出目录: $OUTPUT_DIR/"
    echo "   - frame_XXXX.png  (截图序列)"
    echo "   - game_log.txt    (战斗日志)"
    echo ""
    echo "💡 查看截图: xdg-open $OUTPUT_DIR/frame_0001.png"
    echo "💡 制作GIF:  ffmpeg -framerate 4 -i $OUTPUT_DIR/frame_%04d.png $OUTPUT_DIR/combat.gif"
    exit 0
else
    echo "⚠️ 测试未完全通过，请查看日志"
    echo "   日志: $OUTPUT_DIR/game_log.txt"
    exit 1
fi
