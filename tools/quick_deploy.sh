#!/bin/bash
# 快速部署脚本 - 编译、测试、打包

set -e

PROJECT_DIR="D:/workspace/first-game"
UE5_DIR="$PROJECT_DIR/src_ue5"
BUILD_BAT="D:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat"

echo "=================================="
echo "格斗萌主 - 快速部署"
echo "=================================="
echo ""

# 1. 编译
echo "步骤 1/3: 编译项目..."
"$BUILD_BAT" FirstGameEditor Win64 Development \
    "-Project=$UE5_DIR/FirstGame.uproject" \
    -WaitMutex -NoLiveCoding

if [ $? -eq 0 ]; then
    echo "✅ 编译成功"
else
    echo "❌ 编译失败"
    exit 1
fi

# 2. 测试
echo ""
echo "步骤 2/3: 运行测试..."
# 测试需要编辑器运行，这里跳过
echo "⏭️  跳过（需要编辑器运行）"

# 3. 统计
echo ""
echo "步骤 3/3: 生成统计..."
cd "$PROJECT_DIR"
python -X utf8 tools/generate_asset_report.py

echo ""
echo "=================================="
echo "✅ 部署完成！"
echo "=================================="
