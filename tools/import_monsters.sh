#!/bin/bash
# import_monsters.sh — Import 4 Candy Dungeon monsters into UE5
#
# Usage:
#   bash tools/import_monsters.sh [step]
#
# step:
#   1 = Import SkeletalMesh + animations (default)
#   2 = Create NPR material instances (TBD)
#   3 = Create Animation Blueprints (TBD)
#   all = step 1 for now

set -euo pipefail

UE5_CMD="/home/vipuser/ue5/Engine/Binaries/Linux/UnrealEditor-Cmd"
PROJECT="/home/vipuser/first-game/src_ue5/FirstGame.uproject"
COMMON_ARGS="-stdout -FullStdOutLogOutput -unattended -nosplash -nullrhi"

STEP=${1:-1}

run_step() {
    local step_num="$1"
    local env_var="$2"
    local desc="$3"

    echo "═══════════════════════════════════════════════"
    echo "▶ Step $step_num: $desc"
    echo "═══════════════════════════════════════════════"

    export $env_var=1
    $UE5_CMD "$PROJECT" $COMMON_ARGS 2>&1 | grep -E '\[(MONSTER|INIT|MATERIAL|IMPORT|VERIFY|ABP|ANIMBP|TEST LEVEL)\]|Error:|✅|❌|---' || true
    unset $env_var
}

case $STEP in
    1)
        run_step 1 FIRSTGAME_IMPORT_MONSTERS "Import Monster SkeletalMeshes + Animations"
        ;;
    2)
        run_step 2 FIRSTGAME_SETUP_MONSTERS "Create Materials + AnimBPs"
        ;;
    3)
        run_step 3 FIRSTGAME_SETUP_ANIMBP "Configure BlendSpaces + AnimBP settings"
        ;;
    4)
        run_step 4 FIRSTGAME_TEST_LEVEL "Create Monster Test Level"
        ;;
    all)
        run_step 1 FIRSTGAME_IMPORT_MONSTERS "Import Monster SkeletalMeshes + Animations"
        run_step 2 FIRSTGAME_SETUP_MONSTERS "Create Materials + AnimBPs"
        run_step 3 FIRSTGAME_SETUP_ANIMBP "Configure BlendSpaces + AnimBP settings"
        run_step 4 FIRSTGAME_TEST_LEVEL "Create Monster Test Level"
        ;;
    *)
        echo "Unknown step: $STEP"
        exit 1
        ;;
esac

echo ""
echo "Done."
