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

凭证保存在 `~/.ossutilconfig`，不在仓库中。

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

# 2. 运行安装脚本（自动安装 ossutil + 下载所有资产）
bash tools/oss-assets/setup.sh

# 3. 如果跳过了资产下载，手动执行：
python3 tools/oss-assets/oss-sync.py sync-down
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
