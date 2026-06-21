#!/bin/bash
# ─────────────────────────────────────────────────────────────
# setup.sh — 一键配置 OSS 大文件同步系统
#
# 功能：
#   1. 检查/安装 ossutil
#   2. 配置 OSS 凭证（~/.ossutilconfig）
#   3. 创建 git hooks symlink
#   4. 扫描生成 .large-assets.json
#   5. 首次全量上传到 OSS
#
# 用法：
#   bash tools/oss-assets/setup.sh
# ─────────────────────────────────────────────────────────────

set -e

REPO_ROOT="$(git rev-parse --show-toplevel)"
SCRIPT_DIR="$REPO_ROOT/tools/oss-assets"
HOOKS_DIR="$SCRIPT_DIR/hooks"
GIT_HOOKS_DIR="$REPO_ROOT/.git/hooks"

# 颜色
GREEN="\033[32m"
YELLOW="\033[33m"
RED="\033[31m"
BOLD="\033[1m"
RESET="\033[0m"

info()  { echo -e "${GREEN}[setup]${RESET} $1"; }
warn()  { echo -e "${YELLOW}[setup]${RESET} $1"; }
error() { echo -e "${RED}[setup]${RESET} $1" >&2; }
step()  { echo -e "\n${BOLD}── $1 ──${RESET}"; }

# ── Step 1：检查 ossutil ───────────────────────────────────
step "1/5 检查 ossutil"

if command -v ossutil &>/dev/null; then
    info "ossutil 已安装: $(ossutil --version 2>&1 | head -1)"
else
    warn "ossutil 未安装，尝试安装..."
    OS=$(uname -s)
    ARCH=$(uname -m)

    # 确定平台前缀（linux/darwin）和架构映射
    if [ "$OS" = "Darwin" ]; then
        PLAT="darwin"
    elif [ "$OS" = "Linux" ]; then
        PLAT="linux"
    else
        error "不支持的系统: $OS，请手动安装 ossutil"
        exit 1
    fi

    # 统一架构命名
    case "$ARCH" in
        x86_64)  OSS_ARCH="amd64" ;;
        aarch64|arm64) OSS_ARCH="arm64" ;;
        *) error "不支持的架构: $ARCH，请手动安装 ossutil"; exit 1 ;;
    esac

    OSSUTIL_URL="https://gosspublic.alicdn.com/ossutil/1.7.18/ossutil-v1.7.18-${PLAT}-${OSS_ARCH}.zip"
    info "下载: $OSSUTIL_URL"

    TMPDIR=$(mktemp -d)
    info "下载 ossutil..."
    if command -v curl &>/dev/null; then
        curl -L "$OSSUTIL_URL" -o "$TMPDIR/ossutil.zip" || { error "curl 下载失败"; exit 1; }
    elif command -v wget &>/dev/null; then
        wget -q "$OSSUTIL_URL" -O "$TMPDIR/ossutil.zip" || { error "wget 下载失败"; exit 1; }
    else
        error "需要 curl 或 wget 来下载 ossutil"
        exit 1
    fi

    # 验证下载的文件是 zip（不是 HTML 错误页面）
    if ! file "$TMPDIR/ossutil.zip" | grep -qi zip; then
        error "下载的文件不是有效 zip（可能是 404 页面）"
        echo "  请手动下载: $OSSUTIL_URL"
        exit 1
    fi
    info "下载完成: $(du -h "$TMPDIR/ossutil.zip" | cut -f1)"

    unzip -o "$TMPDIR/ossutil.zip" -d "$TMPDIR" || { error "解压失败"; exit 1; }

    # macOS 上二进制名是 ossutil，Linux 上是 ossutil64
    BINARY=$(find "$TMPDIR" -name "ossutil*" -not -name "*.zip" -type f | head -1)
    if [ -z "$BINARY" ]; then
        error "下载的文件中找不到 ossutil 二进制"
        exit 1
    fi

    # 尝试安装到系统路径
    if sudo cp "$BINARY" /usr/local/bin/ossutil 2>/dev/null; then
        sudo chmod +x /usr/local/bin/ossutil
        info "ossutil 安装到 /usr/local/bin/ossutil"
    else
        # 回退到用户目录
        mkdir -p "$HOME/.local/bin"
        cp "$BINARY" "$HOME/.local/bin/ossutil"
        chmod +x "$HOME/.local/bin/ossutil"
        export PATH="$HOME/.local/bin:$PATH"
        info "ossutil 安装到 ~/.local/bin/ossutil（请确保 PATH 包含此目录）"
    fi

    rm -rf "$TMPDIR"

    if ! command -v ossutil &>/dev/null; then
        error "ossutil 安装失败，请手动安装"
        exit 1
    fi
fi

# ── Step 2：配置 OSS 凭证 ──────────────────────────────────
step "2/5 配置 OSS 凭证"

if [ -f "$HOME/.ossutilconfig" ]; then
    info "ossutil 配置已存在 (~/.ossutilconfig)"
    # 测试连接
    if ossutil ls oss://xiaoxiao-game/ --limited-num 1 &>/dev/null; then
        info "OSS 连接测试成功 ✓"
    else
        warn "OSS 连接测试失败，请检查配置"
        echo "  重新配置: ossutil config"
    fi
else
    info "配置 OSS 凭证..."
    echo ""
    echo "  请输入以下信息（或按 Enter 使用默认值）："
    echo ""
    read -p "  Endpoint [oss-cn-beijing.aliyuncs.com]: " ENDPOINT
    ENDPOINT=${ENDPOINT:-oss-cn-beijing.aliyuncs.com}
    read -p "  AccessKey ID: " AK_ID
    read -s -p "  AccessKey Secret: " AK_SECRET
    echo ""

    if [ -z "$AK_ID" ] || [ -z "$AK_SECRET" ]; then
        error "AccessKey 不能为空"
        exit 1
    fi

    cat > "$HOME/.ossutilconfig" << EOF
[Credentials]
language=EN
endpoint=$ENDPOINT
accessKeyID=$AK_ID
accessKeySecret=$AK_SECRET
EOF
    chmod 600 "$HOME/.ossutilconfig"
    info "凭证已保存到 ~/.ossutilconfig"

    # 测试连接
    if ossutil ls oss://xiaoxiao-game/ --limited-num 1 &>/dev/null; then
        info "OSS 连接测试成功 ✓"
    else
        error "OSS 连接测试失败，请检查配置"
        exit 1
    fi
fi

# ── Step 3：安装 Git Hooks ──────────────────────────────────
step "3/5 安装 Git Hooks"

mkdir -p "$GIT_HOOKS_DIR"

for hook_name in pre-commit post-checkout post-merge; do
    src="$HOOKS_DIR/$hook_name"
    dst="$GIT_HOOKS_DIR/$hook_name"

    if [ ! -f "$src" ]; then
        warn "hook 文件不存在: $src，跳过"
        continue
    fi

    # 确保 hook 脚本有执行权限
    chmod +x "$src"

    # 如果目标已存在且不是我们的 symlink
    if [ -e "$dst" ] && [ ! -L "$dst" ]; then
        # 备份现有 hook
        backup="$dst.backup-$(date +%s)"
        mv "$dst" "$backup"
        info "备份已有 hook: $hook_name → $backup"
    fi

    # 创建 symlink
    ln -sf "$src" "$dst"
    info "hook 已安装: .git/hooks/$hook_name → tools/oss-assets/hooks/$hook_name"
done

# ── Step 4：生成资产清单 ───────────────────────────────────
step "4/5 扫描大 FBX 文件"

info "扫描仓库中的大 FBX 文件（>= 5MB）..."
python3 "$SCRIPT_DIR/oss-sync.py" scan

# ── Step 5：首次上传 ───────────────────────────────────────
step "5/5 首次上传到 OSS"

echo ""
read -p "  是否立即上传所有大 FBX 到 OSS？[Y/n] " CONFIRM
CONFIRM=${CONFIRM:-Y}

if [[ "$CONFIRM" =~ ^[Yy] ]]; then
    info "开始上传..."
    python3 "$SCRIPT_DIR/oss-sync.py" sync-up
    info "首次上传完成！"
else
    warn "跳过上传。稍后手动运行："
    echo "  python3 tools/oss-assets/oss-sync.py sync-up"
fi

# ── 完成 ───────────────────────────────────────────────────
echo ""
echo -e "${GREEN}${BOLD}═══════════════════════════════════════════════════════${RESET}"
echo -e "${GREEN}${BOLD}  ✅ OSS 大文件同步系统配置完成！${RESET}"
echo -e "${GREEN}${BOLD}═══════════════════════════════════════════════════════${RESET}"
echo ""
echo "  📋 常用命令："
echo "     查看同步状态:  python3 tools/oss-assets/oss-sync.py check"
echo "     上传变更:      python3 tools/oss-assets/oss-sync.py sync-up"
echo "     下载缺失:      python3 tools/oss-assets/oss-sync.py sync-down"
echo "     重新扫描:      python3 tools/oss-assets/oss-sync.py scan"
echo ""
echo "  🔧 Git Hooks 已安装："
echo "     pre-commit    → 自动上传 >= 5MB FBX，从 git 中移除"
echo "     post-checkout → 分支切换后下载缺失 FBX"
echo "     post-merge    → merge 后下载新增 FBX"
echo ""
echo "  📖 详细说明: tools/oss-assets/README.md"
echo ""
