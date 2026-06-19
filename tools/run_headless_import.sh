#!/bin/bash
# run_headless_import.sh — UE5 无头资产导入管线
#
# 在 GPU 服务器上执行。使用 -nullrhi 跳过 Vulkan 渲染，
# 在 Editor 模式下通过 -RunPythonScript 执行资产导入。
#
# 用法:
#   bash run_headless_import.sh [step]
#
# step:
#   1 = 导入骨骼网格体 + 贴图
#   2 = 导入动画序列
#   3 = 创建 Animation Blueprint
#   4 = 运行 AnimTest 验证
#   all = 执行全部步骤（默认）

set -euo pipefail

UE5_CMD="/root/autodl-tmp/ue5/Engine/Binaries/Linux/UnrealEditor-Cmd"
PROJECT="/root/autodl-tmp/project/first-game/src_ue5/FirstGame.uproject"
PYTHON_DIR="/root/autodl-tmp/project/first-game/src_ue5/Content/Python"

COMMON_ARGS="-stdout -FullStdOutLogOutput -unattended -nosplash -nullrhi"

STEP=${1:-all}

# ─── 辅助函数 ────────────────────────────────────────────
run_python() {
    local script_name="$1"
    local script_path="${PYTHON_DIR}/${script_name}"

    echo "═══════════════════════════════════════════════"
    echo "▶ Running: ${script_name}"
    echo "═══════════════════════════════════════════════"

    if [ ! -f "$script_path" ]; then
        echo "✗ Script not found: $script_path"
        exit 1
    fi

    su - ueuser -c "${UE5_CMD} ${PROJECT} -RunPythonScript=\"${script_path}\" ${COMMON_ARGS}" 2>&1 | \
        tee "/tmp/ue5_import_$(date +%H%M%S)_${script_name%.py}.log"

    local exit_code=$?
    if [ $exit_code -ne 0 ]; then
        echo "✗ Failed with exit code: $exit_code"
        echo "  Check log: /tmp/ue5_import_*_${script_name%.py}.log"
        return 1
    fi

    echo "✓ ${script_name} completed"
    echo ""
}

# ─── 步骤执行 ────────────────────────────────────────────
case $STEP in
    1|mesh)
        echo "Step 1: Import Skeletal Mesh + Textures"
        run_python "import_skeletal_mesh.py"
        ;;

    2|anim)
        echo "Step 2: Import Animation Sequences"
        run_python "import_all_anims_final.py"
        ;;

    3|abp)
        echo "Step 3: Create Animation Blueprint"
        run_python "create_abp.py"
        ;;

    4|test)
        echo "Step 4: Run Animation Test"
        echo "  Note: This requires a display (VNC/X11) to verify visually"
        echo "  Without display, check the log for errors"
        su - ueuser -c "${UE5_CMD} ${PROJECT} \
            /Game/Maps/TestLevel_AnimTest \
            -game -stdout -nullrhi -nosplash" 2>&1 | \
            tee "/tmp/ue5_animtest_$(date +%H%M%S).log"
        ;;

    all)
        echo "Running full import pipeline..."
        echo ""
        run_python "import_skeletal_mesh.py"
        run_python "import_all_anims_final.py"
        run_python "create_abp.py"
        echo ""
        echo "═══════════════════════════════════════════════"
        echo "✓ Full import pipeline complete!"
        echo "═══════════════════════════════════════════════"
        echo ""
        echo "Next steps:"
        echo "  1. Check logs in /tmp/ue5_import_*.log"
        echo "  2. Connect via VNC to verify assets in UE5 Editor"
        echo "  3. Open TestLevel_AnimTest to test animation playback"
        ;;

    *)
        echo "Usage: $0 [1|2|3|4|all|mesh|anim|abp|test]"
        exit 1
        ;;
esac
