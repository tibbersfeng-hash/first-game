# 格斗萌主 — UE5 Project

Q版2.5D横版格斗闯关游戏，Unreal Engine 5.6 + C++ / Blueprint 混合开发。

## 快速开始

### 前提条件
- UE5.6 已安装（Epic Games Launcher 或源码编译）
- Visual Studio 2022 / Rider / VS Code with C++ 扩展

### 打开项目
1. 用 UE5 Editor 打开 `FirstGame.uproject`
2. 首次打开会自动编译 C++ 模块（约 5-10 分钟）
3. 编译完成后编辑器自动打开

### 命令行编译 (Linux)
```bash
# 生成项目文件
./Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh \
  -project="/path/to/first-game/src_ue5/FirstGame.uproject" -game -engine

# 编译
make FirstGameEditor Linux Development

# 或增量编译（更快）
make FirstGameEditor Linux Development -j$(nproc)
```

### 命令行编译 (Windows)
```bat
Engine\Build\BatchFiles\Build.bat FirstGame Win64 Development -WaitMutex
```

## 项目结构

```
src_ue5/
├── FirstGame.uproject          # 项目文件
├── Config/                      # 引擎配置
├── Content/                     # 资产（Blueprint、模型、贴图）
└── Source/FirstGame/            # C++ 源码
    ├── Public/                  # 头文件
    │   ├── Subsystems/          # SignalBus, CombatData, GameManager
    │   ├── Characters/          # PlayerCharacter, BaseEnemy
    │   ├── Combat/              # HitBox, HurtBox
    │   ├── AI/                  # EnemyAIController
    │   ├── Dungeon/             # DungeonRoom, DungeonFlow
    │   ├── Level/               # LevelBuilder (程序化关卡生成)
    │   ├── UI/                  # HUDWidget
    │   └── DataAssets/          # CharacterDataAsset
    └── Private/                 # 实现文件
```

## 核心架构

- **SignalBus** (GameInstanceSubsystem) — 全局事件广播
- **GAS** (Gameplay Ability System) — 技能/战斗系统
- **DataAsset** — 角色/敌人数据配置
- **2D约束** — CharacterMovement 锁定 Y 轴平面
- **Enhanced Input** — 键盘 + 手柄输入
- **LevelBuilder** — 程序化关卡生成（详见下方）

## LevelBuilder — 一键创建可玩关卡

`ALevelBuilder` 是一个程序化关卡生成 Actor，能把**空关卡**直接变成**可玩关卡**。
它在 `BeginPlay` 里自动构建：

- 场地几何（地板 + 围墙 + 装饰柱，每房间不同配色）
- 灯光（1 方向光 + N 点光）
- 玩家出生点
- DungeonFlow + N 个 DungeonRoom（自动填充敌人）
- HUD Widget（挂到 PlayerController）

### 用法 1: 自动模式（默认）

`AFirstGameGameMode` 默认会 Spawn 一个 `ALevelBuilder`，PIE 即可进入可玩关卡。
默认配置：3 个房间、每房间 3 个敌人、启用 Debug 日志。

### 用法 2: Python 脚本（推荐）

在 UE5 Editor 的 Python 控制台执行：

```python
exec(open(r'/path/to/tools/create_first_level.py').read())
```

脚本会创建 `/Game/Maps/FirstCombatMap` 并自动配置好一切。

### 用法 3: 手动放置

1. 在空关卡中 Place Actor → 搜索 `Level Builder`
2. 在 Details 面板配置：
   - `TotalRooms` — 房间数量
   - `RoomExtent` — 单房间尺寸 (cm)
   - `DefaultEnemyClass` — 默认敌人 Class
   - `HUDWidgetClass` — HUD Widget Blueprint
   - `bDebugMode` — 开启后在 Output Log 打印建造详情
3. PIE 即可

### 自定义配置（推荐做法）

1. 在 Content Browser 右键 LevelBuilder Actor → `Create Blueprint based on this`
2. 命名如 `BP_TestLevelBuilder`
3. 在 Class Defaults 里修改所有参数
4. GameMode Blueprint → Details → `LevelBuilderClass` → 选 `BP_TestLevelBuilder`

之后每次新关卡都会自动使用你的自定义配置。

### 清理

LevelBuilder 在 `EndPlay` 时自动清理所有 Spawn 出的子 Actor。
子 Actor 都带 `LevelBuilder.Child` Tag，便于搜索和手动清理。

## 操作方式

| 操作 | 键盘 | 手柄 |
|------|------|------|
| 移动 | A/D 或 ←/→ | Left Stick |
| 跳跃 | Space | A/Cross |
| 轻攻 | J | X/Square |
| 重攻 | K | Y/Triangle |
| 必杀 | L | B/Circle |
| 闪避 | Shift | LB/L1 |

## 开发流程

1. C++ 代码修改后，在编辑器内按 `Ctrl+Shift+.` 热重载
2. 或使用 Live Coding（Development 配置自动启用）
3. 蓝图修改即时生效，无需编译

## 相关文档

- `../design/gdd/` — 游戏设计文档
- `../docs/architecture/` — 架构决策记录
- `../docs/engine-reference/unreal/` — UE5 参考文档
- `.claude/docs/technical-preferences.md` — 技术偏好与规范
