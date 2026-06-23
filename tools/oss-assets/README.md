# OSS 资产同步系统

**策略：C++ 源码 + 文档 → GitHub | 二进制资产 → 阿里云 OSS**

## 架构

```
GitHub 仓库（轻量，只保留代码+文档）
├── src_ue5/Source/          ← C++ 源码
├── src_ue5/Config/          ← UE5 配置
├── src_ue5/Content/Python/  ← UE5 编辑器脚本
├── design/                  ← 游戏设计文档
├── docs/                    ← 技术文档
├── tools/oss-assets/        ← 本同步系统
├── .large-assets.json       ← 资产清单（路径 + MD5 hash）
└── .git/hooks/              ← symlink → tools/oss-assets/hooks/

阿里云 OSS: oss://xiaoxiao-game/first-game/...
├── src_ue5/Content/**/*.uasset    ← UE5 资产
├── src_ue5/Content/**/*.umap      ← UE5 关卡
├── src_ue5/Content/**/*.fbx       ← 3D 模型/动画
├── src_ue5/Content/**/*.png       ← 贴图
├── docs/**/*.png                  ← 文档截图
└── design/assets/output/3d/**     ← 美术源文件
```

## 走 OSS 的文件类型

| 类型 | 扩展名 |
|------|--------|
| UE5 资产 | `.uasset`, `.umap` |
| 3D 模型 | `.fbx`, `.obj`, `.glb`, `.gltf` |
| 图片/贴图 | `.png`, `.jpg`, `.tga`, `.exr`, `.hdr`, `.psd` |
| 音频 | `.wav`, `.mp3`, `.ogg` |
| 3D 源文件 | `.blend`, `.fbm/` |

## 快速开始

```bash
# 首次配置（安装 ossutil + 配置 hooks + 下载所有资产）
bash tools/oss-assets/setup.sh
```

## 日常使用

```bash
# 查看同步状态
python3 tools/oss-assets/oss-sync.py check

# 上传本地新增/变更的资产
python3 tools/oss-assets/oss-sync.py sync-up

# 下载缺失的资产
python3 tools/oss-assets/oss-sync.py sync-down

# 上传/下载单个文件
python3 tools/oss-assets/oss-sync.py upload path/to/file.uasset
python3 tools/oss-assets/oss-sync.py download src_ue5/Content/Maps/TestLevel.umap
```

## Git Hooks 行为

### pre-commit（commit 前）

1. 检测暂存区中应走 OSS 的二进制文件（被 .gitignore 覆盖但被 `git add -f` 强制添加的）
2. 自动上传到 OSS
3. 从 git 索引中移除（`git rm --cached`，本地保留）
4. 更新 `.large-assets.json` 并加入暂存区
5. 中止本次 commit（提示重新 commit）

> 如果 ossutil 不可用，仅打印警告，不阻塞 commit。

### post-checkout（分支切换后）

扫描 `.large-assets.json`，下载本地缺失的资产文件。

### post-merge（merge/pull 后）

下载 merge 引入的新资产文件。

## OSS 配置

| 项 | 值 |
|---|---|
| Bucket | `xiaoxiao-game` |
| Endpoint | `oss-cn-beijing.aliyuncs.com` |
| 路径前缀 | `first-game/` |
| AccessKey | 从团队获取或从 0.100 服务器复制 |

### 手动配置凭证

创建 `~/.ossutilconfig`（Linux/macOS）或 `%USERPROFILE%\.ossutilconfig`（Windows）：

```ini
[Credentials]
language=EN
endpoint=oss-cn-beijing.aliyuncs.com
accessKeyID=<从团队获取>
accessKeySecret=<从团队获取>
```

> **快速方式**：从 0.100 服务器直接复制凭证文件
> ```bash
> scp root@172.25.0.100:~/.ossutilconfig ~/.ossutilconfig
> ```

### 安装 ossutil

```bash
# Linux
curl -L https://gosspublic.alicdn.com/ossutil/1.7.19/ossutil-v1.7.19-linux-amd64.zip -o /tmp/ossutil.zip
unzip /tmp/ossutil.zip -d /tmp && sudo cp /tmp/ossutil*/ossutil64 /usr/local/bin/ossutil && chmod +x /usr/local/bin/ossutil

# macOS (Intel)
curl -L https://gosspublic.alicdn.com/ossutil/1.7.19/ossutil-v1.7.19-mac-amd64.zip -o /tmp/ossutil.zip
unzip /tmp/ossutil.zip -d /tmp && sudo cp /tmp/ossutil*/ossutil /usr/local/bin/ossutil && chmod +x /usr/local/bin/ossutil

# macOS (Apple Silicon)
curl -L https://gosspublic.alicdn.com/ossutil/1.7.19/ossutil-v1.7.19-mac-arm64.zip -o /tmp/ossutil.zip
unzip /tmp/ossutil.zip -d /tmp && sudo cp /tmp/ossutil*/ossutil /usr/local/bin/ossutil && chmod +x /usr/local/bin/ossutil

# Windows
curl -L https://gosspublic.alicdn.com/ossutil/1.7.19/ossutil-v1.7.19-windows-amd64.zip -o %TEMP%\ossutil.zip
# 解压后将 ossutil64.exe 放入 PATH 目录

# 验证
ossutil --version
ossutil ls oss://xiaoxiao-game/ --limited-num 3
```

> **注意**：Windows 上运行 `oss-sync.py` 时需要设置 UTF-8 环境变量：
> ```cmd
> set PYTHONUTF8=1
> set PYTHONIOENCODING=utf-8
> python tools\oss-assets\oss-sync.py sync-down
> ```

## .large-assets.json

清单文件，记录所有 OSS 资产的路径和 MD5 hash，**提交到 git**。
其他成员通过它知道哪些文件需要从 OSS 下载。

```json
{
  "version": 2,
  "assets": {
    "src_ue5/Content/Materials/M_NPR_Parent.uasset": {
      "md5": "a1b2c3d4...",
      "size": 19453,
      "uploaded_at": "2026-06-22T00:00:00+00:00"
    }
  }
}
```

## 新成员入门

```bash
# 1. Clone 仓库
git clone git@github.com:tibbersfeng-hash/first-game.git
cd first-game

# 2. 从 0.100 服务器复制 OSS 凭证
scp root@172.25.0.100:~/.ossutilconfig ~/.ossutilconfig

# 3. 安装 ossutil（见上方"安装 ossutil"章节）

# 4. 下载所有资源
# Linux/macOS:
python3 tools/oss-assets/oss-sync.py sync-down
# Windows:
set PYTHONUTF8=1 && set PYTHONIOENCODING=utf-8 && python tools\oss-assets\oss-sync.py sync-down

# 5. 验证
python3 tools/oss-assets/oss-sync.py check
```

## 排错

### ossutil 连接失败
```bash
ossutil ls oss://xiaoxiao-game/
cat ~/.ossutilconfig
```

### Hook 不生效
```bash
ls -la .git/hooks/pre-commit
ln -sf ../../tools/oss-assets/hooks/pre-commit .git/hooks/pre-commit
```

### 跳过 hook（紧急情况）
```bash
git commit --no-verify -m "紧急提交"
```
