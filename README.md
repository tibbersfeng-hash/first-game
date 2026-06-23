# 格斗萌主 (First Game)

Q版 3D 第三人称动作游戏，参考崩坏3/原神风格。

## 🎮 项目信息

- **引擎**: Unreal Engine 5.7
- **语言**: C++ (核心系统) + Blueprint (内容/关卡)
- **风格**: 动漫风格化 NPR (Toon Shading)
- **阶段**: P0 原型

## 📁 项目结构

```
first-game/
├── src_ue5/              # UE5 项目
│   ├── Source/           # C++ 源码 (48 cpp + 51 h)
│   ├── Content/          # 游戏资产 (76 uasset + 11 umap + 67 fbx)
│   ├── Config/           # 项目配置
│   └── FirstGame.uproject
├── design/               # 游戏设计文档
├── docs/                 # 技术文档
├── tools/                # 开发工具
│   ├── oss-assets/       # OSS 资源同步
│   └── *.py              # 自动化脚本
├── production/           # 生产状态
├── CLAUDE.md             # AI 助手配置
└── HANDOVER.md           # 项目交接文档
```

## 🚀 快速开始

### 环境要求

- Windows 10/11
- Visual Studio 2022 (MSVC v143)
- Unreal Engine 5.7
- Python 3.8+

### 编译项目

```bash
# 使用 UnrealBuildTool
"D:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" \
  FirstGameEditor Win64 Development \
  -Project=D:\workspace\first-game\src_ue5\FirstGame.uproject

# 或使用批量工具
python tools/batch_operations.py build
```

### 运行测试

```bash
# 运行 101 个自动化测试
python tools/batch_operations.py test
```

### 启动编辑器

```bash
"D:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" \
  D:\workspace\first-game\src_ue5\FirstGame.uproject
```

## 📊 当前状态

### ✅ 已完成

- **C++ 系统** (48 源文件，编译通过)
  - PlayerCharacter + BaseEnemy (GAS 集成)
  - 第三人称相机系统
  - Lock-On 目标锁定
  - ComboManager 连招系统
  - HitBox/HurtBox 碰撞检测
  - 打击顿帧 + 屏幕震动
  - 地牢刷怪系统
  - 音频管理

- **3D 资产**
  - 武僧 Huikong (主角) - 模型 + 7 个动画
  - 4 个怪物 (CandyZombie, Gingerbread, ShadowNinja, ArmoredGum)
  - NPR 材质系统
  - MonsterAnimInstance (C++ 状态机)

- **测试框架**
  - 101 个自动化测试，100% 通过

### ⏳ 待完成

- 描边材质 (Inverted Hull) - 需要 GUI 操作
- AnimBP 状态机 - 已用 C++ 替代方案
- LOD 配置 - 需要 GUI 操作
- 战斗整合测试

## 🛠️ 开发工具

### 批量操作

```bash
python tools/batch_operations.py build    # 编译
python tools/batch_operations.py test     # 测试
python tools/batch_operations.py all      # 全部
```

### 项目验证

```bash
python -X utf8 tools/validate_project.py
```

### 资产报告

```bash
python -X utf8 tools/generate_asset_report.py
```

### OSS 资源同步

```bash
# 下载资源
python -X utf8 tools/oss-assets/oss-sync.py sync-down

# 上传资源
python -X utf8 tools/oss-assets/oss-sync.py sync-up

# 检查状态
python -X utf8 tools/oss-assets/oss-sync.py check
```

## 📝 开发规范

- 提交信息使用 Conventional Commits 格式
- C++ 代码遵循 UE5 编码规范
- 所有新功能必须包含单元测试
- 大型二进制资产通过 OSS 管理

## 🔗 相关链接

- [HANDOVER.md](HANDOVER.md) - 详细项目交接文档
- [CLAUDE.md](CLAUDE.md) - AI 助手配置
- [tools/oss-assets/README.md](tools/oss-assets/README.md) - OSS 使用指南

## 📄 许可证

私有项目 - 格斗萌主 Team
