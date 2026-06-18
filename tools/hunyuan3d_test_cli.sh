#!/bin/bash
# 混元 3D 专业版 API 测试脚本（CLI 版本）
# 用法: ./hunyuan3d_test_cli.sh

set -e

# 配置
REGION="ap-guangzhou"
OUTPUT_DIR="./output/test_hunyuan3d"
PROMPT="Q版三头身气功小武僧角色，光头，蓝色僧衣，红色腰带，圆润可爱的卡通风格，游戏角色，正面全身，白色背景"

# 检查 tccli 是否安装
if ! command -v tccli &> /dev/null; then
    echo "❌ 请先安装 tccli: pip install tccli"
    exit 1
fi

# 检查凭证
if ! tccli configure list &> /dev/null; then
    echo "❌ 请先配置腾讯云凭证:"
    echo "   tccli configure set secretId <你的SecretId>"
    echo "   tccli configure set secretKey <你的SecretKey>"
    echo "   tccli configure set region ap-guangzhou"
    exit 1
fi

echo "=========================================="
echo "🧪 混元 3D API 测试"
echo "=========================================="
echo ""
echo "Prompt: $PROMPT"
echo ""

# 创建输出目录
mkdir -p "$OUTPUT_DIR"

# 1. 提交任务
echo "📝 [1/3] 提交生成任务..."
RESPONSE=$(tccli ai3d SubmitHunyuanTo3DProJob \
    --region "$REGION" \
    --cli-unfold-stdout \
    --Prompt "$PROMPT" \
    --Model "3.0" \
    --EnablePBR true \
    --FaceCount 100000 \
    --GenerateType "Normal" \
    --ResultFormat "GLB" 2>&1)

echo "$RESPONSE"

# 提取 JobId
JOB_ID=$(echo "$RESPONSE" | grep -o '"JobId"[[:space:]]*:[[:space:]]*"[^"]*"' | cut -d'"' -f4)

if [ -z "$JOB_ID" ]; then
    echo "❌ 提交失败"
    exit 1
fi

echo "✅ JobId: $JOB_ID"
echo ""

# 2. 轮询查询状态
echo "⏳ [2/3] 等待生成完成..."
MAX_WAIT=600
INTERVAL=15
ELAPSED=0

while [ $ELAPSED -lt $MAX_WAIT ]; do
    QUERY_RESULT=$(tccli ai3d QueryHunyuanTo3DProJob \
        --region "$REGION" \
        --cli-unfold-stdout \
        --JobId "$JOB_ID" 2>&1)

    STATUS=$(echo "$QUERY_RESULT" | grep -o '"Status"[[:space:]]*:[[:space:]]*"[^"]*"' | cut -d'"' -f4)

    case "$STATUS" in
        "WAIT")
            echo -n "  ⏸️  排队中... (${ELAPSED}秒)\r"
            ;;
        "RUN")
            echo -n "  ⚙️  生成中... (${ELAPSED}秒)\r"
            ;;
        "DONE")
            echo ""
            echo "✅ 生成完成！耗时 ${ELAPSED}秒"
            echo ""
            echo "$QUERY_RESULT"
            break
            ;;
        "FAIL")
            echo ""
            echo "❌ 生成失败:"
            echo "$QUERY_RESULT"
            exit 1
            ;;
        *)
            echo ""
            echo "⚠️  未知状态: $STATUS"
            echo "$QUERY_RESULT"
            exit 1
            ;;
    esac

    sleep $INTERVAL
    ELAPSED=$((ELAPSED + INTERVAL))
done

if [ $ELAPSED -ge $MAX_WAIT ]; then
    echo ""
    echo "❌ 超时 (${MAX_WAIT}秒)"
    exit 1
fi

# 3. 下载结果
echo ""
echo "📥 [3/3] 下载结果..."

# 提取下载 URL（简化版，实际需要解析 JSON）
# 这里提示用户手动下载
echo ""
echo "=========================================="
echo "✅ 测试完成！"
echo "=========================================="
echo ""
echo "积分消耗: 查看上方 ResultCreditConsumed"
echo ""
echo "下载链接: 查看上方 ResultFile3Ds[].Url"
echo ""
echo "手动下载:"
echo "  curl -o model.glb.zip '<从上面复制URL>'"
echo "  unzip model.glb.zip"
echo ""
echo "或访问预览图: ResultFile3Ds[].PreviewImageUrl"
