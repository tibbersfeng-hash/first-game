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
    │   ├── UI/                  # HUDWidget
    │   ── DataAssets/          # CharacterDataAsset
    └── Private/                 # 实现文件
```

## 核心架构

- **SignalBus** (GameInstanceSubsystem) — 全局事件广播
- **GAS** (Gameplay Ability System) — 技能/战斗系统
- **DataAsset** — 角色/敌人数据配置
- **2D约束** — CharacterMovement 锁定 Y 轴平面
- **Enhanced Input** — 键盘 + 手柄输入

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
