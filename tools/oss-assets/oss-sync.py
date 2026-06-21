#!/usr/bin/env python3
"""
OSS Asset Sync — 将二进制资产（图片、模型、UE5 Content）同步到阿里云 OSS

策略：C++ 源码 + 文档留在 GitHub，所有二进制资产走 OSS。

用法：
    python3 oss-sync.py upload <file>      # 上传单个文件到 OSS
    python3 oss-sync.py download <file>    # 从 OSS 下载单个文件
    python3 oss-sync.py sync-up            # 上传所有本地新增/变更的资产
    python3 oss-sync.py sync-down          # 下载所有清单中缺失的资产
    python3 oss-sync.py check              # 显示同步状态
    python3 oss-sync.py scan               # 扫描并生成 .large-assets.json
    python3 oss-sync.py upload-staged <file1> [file2 ...]  # 批量上传（git hook 用）

环境变量（可选覆盖默认值）：
    OSS_BUCKET      Bucket 名称（默认 xiaoxiao-game）
    OSS_ENDPOINT    区域端点（默认 oss-cn-beijing.aliyuncs.com）
    OSS_PREFIX      OSS 路径前缀（默认 first-game/）
"""

import hashlib
import json
import os
import subprocess
import sys
from datetime import datetime, timezone

# ── 配置 ─────────────────────────────────────────────────────────────────────
OSS_BUCKET = os.environ.get("OSS_BUCKET", "xiaoxiao-game")
OSS_ENDPOINT = os.environ.get("OSS_ENDPOINT", "oss-cn-beijing.aliyuncs.com")
OSS_PREFIX = os.environ.get("OSS_PREFIX", "first-game/")

# 走 OSS 的文件扩展名（全部小写）
ASSET_EXTENSIONS = {
    # UE5 Content
    ".uasset", ".umap",
    # 3D 模型 / 动画
    ".fbx", ".obj", ".gltf", ".glb",
    # 图片 / 贴图
    ".png", ".jpg", ".jpeg", ".tga", ".bmp", ".exr", ".hdr", ".psd", ".tif", ".tiff",
    # 音频
    ".wav", ".mp3", ".ogg", ".flac",
    # 视频
    ".mp4", ".mov", ".webm", ".avi",
    # 3D 源文件
    ".blend", ".max", ".maya", ".zpr",
    # FBX 材质子目录中的文件（.fbm/）
    # 这些文件通过路径中包含 .fbm/ 来识别
}

# 跳过的目录（UE5 缓存 / 构建产物 / 系统目录）
SKIP_DIRS = {
    ".git", ".github",
    "DerivedDataCache", "Intermediate", "Saved", "Binaries",
    "node_modules", "__pycache__", ".venv", "venv",
    ".claude",  # Claude 配置
}

# ── 路径 ─────────────────────────────────────────────────────────────────────
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def _get_repo_root():
    """通过 git 获取仓库根目录"""
    try:
        result = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            capture_output=True, text=True, cwd=SCRIPT_DIR
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except Exception:
        pass
    return os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))


REPO_ROOT = _get_repo_root()
MANIFEST_PATH = os.path.join(REPO_ROOT, ".large-assets.json")

# ── 颜色输出 ──────────────────────────────────────────────────────────────────
GREEN = "\033[32m"
YELLOW = "\033[33m"
RED = "\033[31m"
CYAN = "\033[36m"
RESET = "\033[0m"
BOLD = "\033[1m"


def info(msg):
    print(f"{GREEN}[OSS]{RESET} {msg}")


def warn(msg):
    print(f"{YELLOW}[OSS]{RESET} {msg}")


def error(msg):
    print(f"{RED}[OSS]{RESET} {msg}", file=sys.stderr)


# ── 工具函数 ──────────────────────────────────────────────────────────────────

def is_asset_file(relpath):
    """判断文件是否为二进制资产（应走 OSS）"""
    lower = relpath.lower()
    # .fbm/ 目录中的文件（FBX 材质子目录）
    if "/.fbm/" in lower or lower.endswith(".fbm"):
        return True
    _, ext = os.path.splitext(lower)
    return ext in ASSET_EXTENSIONS


def check_ossutil():
    """检查 ossutil 是否可用"""
    try:
        subprocess.run(["ossutil", "--version"], capture_output=True, check=True)
        return True
    except (FileNotFoundError, subprocess.CalledProcessError):
        return False


def file_md5(filepath):
    """计算文件 MD5 hash"""
    h = hashlib.md5()
    with open(filepath, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


def load_manifest():
    """加载 .large-assets.json"""
    if not os.path.exists(MANIFEST_PATH):
        return {"version": 2, "assets": {}}
    with open(MANIFEST_PATH, "r") as f:
        return json.load(f)


def save_manifest(manifest):
    """保存 .large-assets.json"""
    with open(MANIFEST_PATH, "w") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)
        f.write("\n")


def find_assets():
    """扫描仓库中所有二进制资产文件"""
    results = []
    for root, dirs, files in os.walk(REPO_ROOT):
        # 过滤跳过的目录
        dirs[:] = [d for d in dirs if d not in SKIP_DIRS and not d.startswith(".")]

        for fname in files:
            fpath = os.path.join(root, fname)
            relpath = os.path.relpath(fpath, REPO_ROOT)

            if not is_asset_file(relpath):
                continue

            try:
                size = os.path.getsize(fpath)
            except OSError:
                continue

            results.append((relpath, fpath, size))
    return results


def oss_path(relpath):
    """将本地相对路径转换为 OSS 完整路径"""
    return f"oss://{OSS_BUCKET}/{OSS_PREFIX}{relpath}"


def oss_check_exists(relpath):
    """检查文件是否在 OSS 上存在，返回 (exists, remote_md5)"""
    oss_url = oss_path(relpath)
    try:
        result = subprocess.run(
            ["ossutil", "stat", oss_url],
            capture_output=True, text=True, timeout=30
        )
        if result.returncode != 0:
            return False, None
        for line in result.stdout.splitlines():
            if "etag" in line.lower() or "ETag" in line:
                etag = line.split(":", 1)[1].strip().strip('"').lower()
                return True, etag
        return True, None
    except (subprocess.TimeoutExpired, Exception) as e:
        error(f"OSS 检查失败: {e}")
        return False, None


def format_size(size_bytes):
    """格式化文件大小"""
    if size_bytes >= 1024 * 1024:
        return f"{size_bytes / 1024 / 1024:.1f}MB"
    elif size_bytes >= 1024:
        return f"{size_bytes / 1024:.0f}KB"
    return f"{size_bytes}B"


# ── 核心命令 ──────────────────────────────────────────────────────────────────

def cmd_upload(filepath):
    """上传单个文件到 OSS 并更新清单"""
    if not os.path.exists(filepath):
        error(f"文件不存在: {filepath}")
        return False

    if not check_ossutil():
        error("ossutil 未安装或不可用。运行: tools/oss-assets/setup.sh")
        return False

    relpath = os.path.relpath(filepath, REPO_ROOT)
    local_md5 = file_md5(filepath)
    size = os.path.getsize(filepath)

    info(f"上传中: {relpath} ({format_size(size)})...")

    result = subprocess.run(
        ["ossutil", "cp", filepath, oss_path(relpath), "-f"],
        capture_output=True, text=True, timeout=600
    )

    if result.returncode != 0:
        error(f"上传失败:\n{result.stderr}")
        return False

    manifest = load_manifest()
    manifest["assets"][relpath] = {
        "md5": local_md5,
        "size": size,
        "uploaded_at": datetime.now(timezone.utc).isoformat()
    }
    save_manifest(manifest)

    info(f"上传成功: {relpath}")
    return True


def cmd_upload_staged(filepaths):
    """批量上传文件（pre-commit hook 调用）"""
    if not check_ossutil():
        error("ossutil 未安装或不可用")
        return False

    success = True
    manifest = load_manifest()
    now = datetime.now(timezone.utc).isoformat()

    for filepath in filepaths:
        if not os.path.exists(filepath):
            error(f"文件不存在: {filepath}")
            success = False
            continue

        relpath = os.path.relpath(filepath, REPO_ROOT)
        local_md5 = file_md5(filepath)
        size = os.path.getsize(filepath)

        record = manifest["assets"].get(relpath)
        if record and record.get("md5") == local_md5:
            info(f"跳过（未变更）: {relpath}")
            continue

        info(f"上传中: {relpath} ({format_size(size)})...")
        result = subprocess.run(
            ["ossutil", "cp", filepath, oss_path(relpath), "-f"],
            capture_output=True, text=True, timeout=600
        )

        if result.returncode != 0:
            error(f"上传失败: {relpath}\n{result.stderr}")
            success = False
            continue

        manifest["assets"][relpath] = {
            "md5": local_md5,
            "size": size,
            "uploaded_at": now
        }
        info(f"上传成功: {relpath}")

    save_manifest(manifest)
    return success


def cmd_download(relpath):
    """从 OSS 下载单个文件"""
    if not check_ossutil():
        error("ossutil 未安装或不可用")
        return False

    filepath = os.path.join(REPO_ROOT, relpath)

    exists, _ = oss_check_exists(relpath)
    if not exists:
        error(f"OSS 上不存在: {relpath}")
        return False

    info(f"下载中: {relpath}...")
    os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)

    result = subprocess.run(
        ["ossutil", "cp", oss_path(relpath), filepath, "-f"],
        capture_output=True, text=True, timeout=600
    )

    if result.returncode != 0:
        error(f"下载失败: {result.stderr}")
        return False

    manifest = load_manifest()
    record = manifest["assets"].get(relpath)
    if record:
        downloaded_md5 = file_md5(filepath)
        if downloaded_md5 != record.get("md5"):
            error(f"下载文件 hash 不匹配: {relpath}")
            os.remove(filepath)
            return False

    info(f"下载成功: {relpath}")
    return True


def cmd_sync_up():
    """扫描本地所有资产，上传新增/变更的"""
    if not check_ossutil():
        error("ossutil 未安装或不可用。运行: tools/oss-assets/setup.sh")
        sys.exit(1)

    files = find_assets()
    manifest = load_manifest()
    now = datetime.now(timezone.utc).isoformat()
    uploaded = 0
    skipped = 0

    for relpath, filepath, size in files:
        local_md5 = file_md5(filepath)
        record = manifest["assets"].get(relpath)

        if record and record.get("md5") == local_md5:
            exists, _ = oss_check_exists(relpath)
            if exists:
                skipped += 1
                continue
            warn(f"OSS 缺失，重新上传: {relpath}")

        info(f"上传: {relpath} ({format_size(size)})...")
        result = subprocess.run(
            ["ossutil", "cp", filepath, oss_path(relpath), "-f"],
            capture_output=True, text=True, timeout=600
        )

        if result.returncode != 0:
            error(f"失败: {relpath}")
            continue

        manifest["assets"][relpath] = {
            "md5": local_md5,
            "size": size,
            "uploaded_at": now
        }
        uploaded += 1

    save_manifest(manifest)
    info(f"sync-up 完成: 上传 {uploaded} 个，跳过 {skipped} 个（未变更）")


def cmd_sync_down():
    """下载清单中所有缺失或过期的文件"""
    if not check_ossutil():
        error("ossutil 未安装或不可用")
        sys.exit(1)

    manifest = load_manifest()
    downloaded = 0
    up_to_date = 0
    failed = 0

    for relpath, record in manifest["assets"].items():
        filepath = os.path.join(REPO_ROOT, relpath)

        if os.path.exists(filepath):
            local_md5 = file_md5(filepath)
            if local_md5 == record.get("md5"):
                up_to_date += 1
                continue

        info(f"下载: {relpath} ({format_size(record.get('size', 0))})...")
        os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)

        result = subprocess.run(
            ["ossutil", "cp", oss_path(relpath), filepath, "-f"],
            capture_output=True, text=True, timeout=600
        )

        if result.returncode != 0:
            error(f"下载失败: {relpath}")
            failed += 1
            continue

        if os.path.exists(filepath):
            downloaded_md5 = file_md5(filepath)
            if downloaded_md5 == record.get("md5"):
                downloaded += 1
            else:
                error(f"hash 验证失败: {relpath}")
                os.remove(filepath)
                failed += 1

    info(f"sync-down 完成: 下载 {downloaded} 个，已有 {up_to_date} 个，失败 {failed} 个")


def cmd_check():
    """显示同步状态"""
    manifest = load_manifest()
    local_files = find_assets()

    manifest_paths = set(manifest["assets"].keys())
    local_paths = {r for r, _, _ in local_files}

    only_oss = manifest_paths - local_paths
    only_local = local_paths - manifest_paths
    both = manifest_paths & local_paths

    changed = []
    up_to_date = []
    for relpath in both:
        filepath = os.path.join(REPO_ROOT, relpath)
        local_md5 = file_md5(filepath)
        record_md5 = manifest["assets"][relpath].get("md5")
        if local_md5 != record_md5:
            changed.append(relpath)
        else:
            up_to_date.append(relpath)

    # 按类型统计
    ext_counts = {}
    for relpath in manifest_paths:
        _, ext = os.path.splitext(relpath.lower())
        if "/.fbm/" in relpath.lower():
            ext = ".fbm/*"
        ext_counts[ext] = ext_counts.get(ext, 0) + 1

    print(f"\n{BOLD}═══ OSS 资产同步状态 ═══{RESET}")
    print(f"  清单条目:   {len(manifest_paths)}")
    print(f"  本地资产:   {len(local_paths)}")
    print(f"  同步完成:   {GREEN}{len(up_to_date)}{RESET}")

    if ext_counts:
        print(f"\n  {CYAN}清单类型分布:{RESET}")
        for ext, count in sorted(ext_counts.items(), key=lambda x: -x[1]):
            print(f"    {ext:12s} {count}")

    if changed:
        print(f"\n  本地已变更: {YELLOW}{len(changed)}{RESET}")
        for p in changed[:10]:
            print(f"    {YELLOW}~{RESET} {p}")
        if len(changed) > 10:
            print(f"    ... 还有 {len(changed) - 10} 个")
    if only_local:
        print(f"\n  仅本地:     {YELLOW}{len(only_local)}{RESET}")
        for p in sorted(only_local)[:10]:
            print(f"    {YELLOW}+{RESET} {p}")
        if len(only_local) > 10:
            print(f"    ... 还有 {len(only_local) - 10} 个")
    if only_oss:
        print(f"\n  仅 OSS:     {RED}{len(only_oss)}{RESET}")
        for p in sorted(only_oss)[:10]:
            print(f"    {RED}-{RESET} {p}")
        if len(only_oss) > 10:
            print(f"    ... 还有 {len(only_oss) - 10} 个")

    if not changed and not only_local and not only_oss:
        print(f"\n  {GREEN}✓ 所有文件同步完成{RESET}\n")
    else:
        print(f"\n  运行 {BOLD}python3 tools/oss-assets/oss-sync.py sync-up{RESET}  上传本地变更")
        print(f"  运行 {BOLD}python3 tools/oss-assets/oss-sync.py sync-down{RESET} 下载缺失文件\n")


def cmd_scan():
    """扫描所有资产生成/覆盖 .large-assets.json（不上传）"""
    files = find_assets()
    manifest = {"version": 2, "assets": {}}
    now = datetime.now(timezone.utc).isoformat()

    for relpath, filepath, size in files:
        md5 = file_md5(filepath)
        manifest["assets"][relpath] = {
            "md5": md5,
            "size": size,
            "uploaded_at": now
        }

    save_manifest(manifest)

    # 按扩展名统计
    ext_counts = {}
    for relpath, _, size in files:
        _, ext = os.path.splitext(relpath.lower())
        if "/.fbm/" in relpath.lower():
            ext = ".fbm/*"
        if ext not in ext_counts:
            ext_counts[ext] = {"count": 0, "size": 0}
        ext_counts[ext]["count"] += 1
        ext_counts[ext]["size"] += size

    info(f"扫描完成: 发现 {len(files)} 个资产文件")
    for ext, data in sorted(ext_counts.items(), key=lambda x: -x[1]["size"]):
        info(f"  {ext:12s} {data['count']:3d} 个  {format_size(data['size'])}")
    info(f"清单已保存: {MANIFEST_PATH}")
    total_size = sum(s for _, _, s in files)
    info(f"总计大小: {format_size(total_size)}")


# ── 主入口 ────────────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    cmd = sys.argv[1]

    if cmd == "upload":
        if len(sys.argv) < 3:
            error("用法: oss-sync.py upload <file>")
            sys.exit(1)
        ok = cmd_upload(sys.argv[2])
        sys.exit(0 if ok else 1)

    elif cmd == "upload-staged":
        if len(sys.argv) < 3:
            error("用法: oss-sync.py upload-staged <file1> [file2 ...]")
            sys.exit(1)
        ok = cmd_upload_staged(sys.argv[2:])
        sys.exit(0 if ok else 1)

    elif cmd == "download":
        if len(sys.argv) < 3:
            error("用法: oss-sync.py download <relpath>")
            sys.exit(1)
        ok = cmd_download(sys.argv[2])
        sys.exit(0 if ok else 1)

    elif cmd == "sync-up":
        cmd_sync_up()

    elif cmd == "sync-down":
        cmd_sync_down()

    elif cmd == "check":
        cmd_check()

    elif cmd == "scan":
        cmd_scan()

    else:
        error(f"未知命令: {cmd}")
        print(__doc__)
        sys.exit(1)


if __name__ == "__main__":
    main()
