#!/bin/bash
# run_automation_tests.sh — Run FirstGame automation tests on GPU server
set -euo pipefail

UE5="/home/vipuser/ue5/Engine/Binaries/Linux/UnrealEditor-Cmd"
PROJECT="/home/vipuser/first-game/src_ue5/FirstGame.uproject"
LOG="/tmp/ue5_automation.log"

# Kill any existing UE5 processes
pkill -9 -f UnrealEditor 2>/dev/null || true
sleep 3

echo "Starting automation tests..."
echo "Log file: $LOG"

# Run tests using -RunAutomationTest flag
timeout 180 "$UE5" "$PROJECT" \
    -nullrhi \
    -stdout \
    -FullStdOutLogOutput \
    -unattended \
    -nosplash \
    -RunAutomationTest=FirstGame.* \
    > "$LOG" 2>&1 &

PID=$!
echo "PID: $PID"

# Wait and monitor
for i in $(seq 1 60); do
    sleep 3
    if ! kill -0 $PID 2>/dev/null; then
        echo "Process finished at iteration $i"
        break
    fi
    LINES=$(wc -l < "$LOG" 2>/dev/null || echo 0)
    echo "[$((i*3))s] $LINES lines, still running..."
done

# Kill if still running
kill $PID 2>/dev/null || true
sleep 2

echo ""
echo "=== TEST RESULTS ==="
grep -E "LogAutomation|Condition|PASS|FAIL|passed|failed|Finished|Total|Summary" "$LOG" || echo "(no test result lines found)"
echo ""
echo "=== LAST 20 LINES ==="
tail -20 "$LOG"
