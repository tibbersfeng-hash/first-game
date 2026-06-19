# first-game 项目交接文档

> 最后更新: 2026-06-19
> 项目路径: `/home/claude/.frontend/first-game`
> UE5 源码: `src_ue5/`
> 仓库: `git@github.com:tibbersfeng-hash/first-game.git` (SSH, HTTPS 被防火墙拦截)

---

## 项目概况

**格斗萌主** — UE5.7 3D 第三人称动作游戏，参考崩坏3/原神
**引擎**: Unreal Engine 5.7.4 | **风格**: 动漫风格化 NPR (Toon Shading)
**角色**: 武僧 Huikong (6.67 头身, 40k 面)

---

## 当前状态 (2026-06-19)

### ✅ 已完成

#### 代码系统 (48 C++ 源文件, 编译通过)
- PlayerCharacter + BaseEnemy (GAS 集成)
- 第三人称相机系统 (ADR-008, SpringArm + CameraController)
- Lock-On 目标锁定 (ADR-009)
- ComboManager 连招系统 (ADR-007)
- HitBox/HurtBox 碰撞检测
- HitStopManager (打击顿帧) + HitCameraShake (屏幕震动)
- CharacterStatsComponent 数值成长
- DungeonFlow + DungeonRoom + WaveManager (地牢刷怪)
- DoorActor (通关门) + LootItem (战利品)
- HUDWidget + DamageNumberWidget + DeathWidget (SignalBus 驱动)
- EnemyAIController + BehaviorTree (Chase/Detect/Attack)
- LevelBuilder 程序化关卡生成
- AudioManagerSubsystem 音频管理
- AnimTestActor 动画测试 (AnimationSingleNode 模式)

#### 测试框架 (17 个测试文件, 编译通过)
- FirstGameTests 模块已加入 Editor Target
- 覆盖: 战斗/连招/碰撞/锁定/数值/信号/地牢/音频/敌人AI
- 72+ 断言，全部使用 `EAutomationTestFlags::EditorContext`

#### 3D 资产
- 武僧模型 FBX (重拓扑 40k 面, 保留面部/服装细节)
- 混元绑骨 + 7 个预设动画:
  - AM_Huikong_Idle_01/02, Walk_01, Run_01, LightAttack_01, HitReaction, Landing
- ABP_Huikong 动画蓝图已创建
- 贴图: BaseColor/Normal/Metallic/Roughness (PBR)

#### 基础设施
- GPU 服务器: RTX 3090 + UE5.7.4 + Vulkan SM5 ✅
- 编译: `libUnrealEditor-FirstGame.so` (1.3MB) + `libUnrealEditor-FirstGameTests.so` (290KB)
- 动画测试: AnimTestActor 成功循环播放 Idle→Walk→Run
- Git: `git fetch --filter=blob:limit=100k` 绕过大型二进制
- GitHub API 推送: 绕过慢速 SSH，直接通过 API 创建 blob/tree/commit

### ⚠️ 已知问题

#### 1. Editor 关闭崩溃 (Signal 11)
- **位置**: `libUnrealEditor-UnrealEd.so` → `AssetEditorSubsystem::FindEditorsForAssetAndSubObjects()`
- **触发**: 关闭 Editor 或结束 PIE 时
- **影响**: 无法在 Editor 中正常关闭/保存
- **规避**: 使用 `-game` 模式运行（不触发 AssetEditor 路径）
- **修复方向**: 需要调试 crash 栈，可能与 MCP Automation Bridge 插件有关

#### 2. Vulkan 渲染在 VNC 中不可见
- TigerVNC 无法捕获 Vulkan 渲染的帧缓冲
- `x11grab` 截屏结果为黑色
- **现状**: 通过 UE5 日志确认渲染正常，无法远程截屏验证视觉效果
- **修复方向**: 使用 UE5 内置 `HighResShot` 命令或 Parsec 串流

#### 3. 本地缺少大型二进制资产
- 本地 git 仓库因网络限制（~500MB FBX）无法 `git pull`
- 已通过 GitHub API 同步文本文件
- GPU 服务器有完整资产

---

## GPU 云服务器

| 项目 | 值 |
|------|-----|
| **SSH** | `ssh gpu` → `vipuser@180.127.11.169:22112` |
| **项目路径** | `/home/vipuser/first-game/` |
| **GPU** | NVIDIA RTX 3090 24GB (Driver 580.159.03) |
| **CPU** | Intel Xeon Gold 6248R @ 3.00GHz |
| **内存** | 47GB |
| **OS** | Ubuntu 24.04 LTS (kernel 6.8.0-31-generic) |
| **UE5** | `/home/vipuser/ue5/` (95GB Installed Build) |
| **VNC** | TigerVNC :1 (port 5901, localhost only) |
| **DRM** | ✅ `/dev/dri/card0`, `renderD128` 可用 |
| **Vulkan** | ✅ `nvidia_icd.json` 注册，SM5 支持 |

### 常用命令

```bash
# 编译
cd /home/vipuser/first-game/src_ue5 && make FirstGameEditor

# 编辑器 (VNC 显示)
export DISPLAY=:1
~/ue5/Engine/Binaries/Linux/UnrealEditor FirstGame.uproject -stdout -nosplash

# 游戏模式测试
~/ue5/Engine/Binaries/Linux/UnrealEditor FirstGame.uproject /Game/Maps/TestLevel_AnimTest -game -stdout

# 无头模式 (无显示)
~/ue5/Engine/Binaries/Linux/UnrealEditor-Cmd FirstGame.uproject -nullrhi -stdout -unattended

# Python 脚本导入资产
~/ue5/Engine/Binaries/Linux/UnrealEditor-Cmd FirstGame.uproject \
  -RunPythonScript="Content/Python/import_skeletal_mesh.py" \
  -stdout -FullStdOutLogOutput -unattended -nosplash -nullrhi
```

### 资产位置

```
Content/Characters/Huikong/
├── Mesh/
│   ├── SKM_Huikong.uasset        # 骨骼网格体 (UE5)
│   ├── SKM_Huikong_Skeleton.uasset
│   ├── SKM_Huikong_Base.fbx       # 重拓扑版本 (40k 面)
│   ├── model_3.fbx                # 原始版本 (150k 面)
│   └── model_3.fbm/               # PBR 贴图
├── Animations/
│   ├── AM_Huikong_Idle_01.uasset
│   ├── AM_Huikong_Idle_02.uasset
│   ├── AM_Huikong_Walk_01.uasset
│   ├── AM_Huikong_Run_01.uasset
│   ├── AM_Huikong_LightAttack_01.uasset
│   ├── AM_Huikong_HitReaction.uasset
│   └── AM_Huikong_Landing.uasset
├── ABP/
│   └── ABP_Huikong.uasset         # 动画蓝图
└── _old_anim/                     # 中文命名原始 FBX (已弃用)
```

### 测试关卡

- `Content/Maps/TestLevel_AnimTest.umap` — 动画测试 (AnimTestActor)
- `Content/Maps/HeadlessPIETest.umap` — 无头 PIE 测试

---

## UE5 项目结构

```
src_ue5/
├── FirstGame.uproject
├── Config/                    # Engine.ini, Game.ini, Input.ini
├── Source/
│   ├── FirstGame/             # 主模块 (48 cpp/h)
│   │   ├── FirstGame.Build.cs
│   │   ├── Private/           # AI/Camera/Characters/Combat/Dungeon/...
│   │   └── Public/
│   ├── FirstGameEditor.Target.cs   # Editor 编译目标 (含 FirstGameTests)
│   ├── FirstGame.Target.cs         # Game 编译目标
│   └── FirstGameTests/        # 测试模块 (17 测试文件)
│       ├── FirstGameTests.Build.cs
│       └── Private/Tests/
├── Content/
│   ├── Characters/Huikong/    # 武僧 3D 资产
│   ├── Maps/                  # 测试关卡
│   ── Python/                # UE5 Python 脚本 (资产导入)
└── tools/                     # 工具脚本
```

### 关键模块依赖

```
FirstGame.Build.cs:
  Public: Core, CoreUObject, Engine, InputCore, EnhancedInput,
          GameplayAbilities, GameplayTags, GameplayTasks,
          UMG, Slate, SlateCore, AIModule

FirstGameTests (UncookedOnly):
  Public: Core, CoreUObject, Engine, FirstGame,
          GameplayAbilities, GameplayTags, GameplayTasks
  Private: UnrealEd, Blutility
```

---

## Git 工作流

### 代码同步 (本地 ↔ GPU 服务器)
```bash
# 本地拉取最新 (跳过大型二进制)
git fetch origin --filter=blob:limit=100k

# 文本文件通过 scp 同步
scp src_ue5/Source/FirstGame/Private/*.cpp gpu:/home/vipuser/first-game/src_ue5/Source/FirstGame/Private/

# 推送到 GitHub 使用 API (绕过慢速 SSH)
# 见 tools/ 目录中的 GitHub API 脚本模式
```

### 提交规范
- Conventional commits: `feat:`, `fix:`, `chore:`, `test:`, `docs:`, `refactor:`
- 编译修复: 记录 UE5 API 变更 (如 `ApplicationContextMask` → `EditorContext`)

---

## P0 原型进度

| 周次 | 主题 | 状态 |
|------|------|------|
| Week 1 | AI 3D 生成 + Blender 修整 | ✅ 完成 |
| Week 2 | 贴图 + 绑骨 + 基础动画 | ✅ 完成 |
| Week 3 | 战斗动画 + NPR + 整合 | ⏳ 待 Editor 崩溃修复 |

### Week 3 待办
- [ ] 修复 AssetEditorSubsystem 崩溃 (或绕过)
- [ ] 完善 Animation Blueprint (BlendSpace + State Machine)
- [ ] 实现 NPR 卡通渲染材质 (2-3 tone + Rim Light + Outline)
- [ ] 整合测试 (移动 + 攻击 + 锁定 + 相机)
- [ ] P0 评审

---

## 新会话快速启动

```bash
# 1. 连接 GPU 服务器
ssh gpu

# 2. 编译
cd /home/vipuser/first-game/src_ue5 && make FirstGameEditor

# 3. 测试动画
export DISPLAY=:1
~/ue5/Engine/Binaries/Linux/UnrealEditor FirstGame.uproject \
  /Game/Maps/TestLevel_AnimTest -game -stdout -nosplash

# 4. 本地开发 (文本文件)
scp gpu:/home/vipuser/first-game/src_ue5/Source/FirstGame/Private/*.cpp src_ue5/Source/FirstGame/Private/
```
