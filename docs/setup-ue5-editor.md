# UE5 编辑器搭建指南 — 格斗萌主

> 本文档指导如何在 UE5 编辑器中完成 Blueprint 资产创建和测试 Level 搭建。
> 前提：`src_ue5/` 已成功编译。

---

## Step 1: 打开项目

1. 启动 Unreal Engine 5.6
2. 打开 `src_ue5/FirstGame.uproject`
3. 首次打开自动编译 C++（等待完成）

---

## Step 2: 创建 DataAsset（角色数据）

### 方法 A：手动创建（推荐，编辑器可视化）

1. **Content Browser** → 右键 → **Miscellaneous → Data Asset**
2. 选择 `CharacterDataAsset` 类
3. 命名为 `DA_Huikong`
4. 在 Details 面板填写：

| 字段 | 慧空值 | 糖糖值 | 小鬼丸值 |
|------|--------|--------|----------|
| CharacterId | Huikong | Tangtang | Kiguemaru |
| DisplayName | 慧空 · 气功小武僧 | 糖糖 · 糖果魔导师 | 小鬼丸 · 萌鬼狂战士 |
| MaxHealth | 100 | 80 | 130 |
| MaxEnergy | 100 | 120 | 80 |
| MoveSpeed | 320 | 280 | 260 |
| JumpForce | 650 | 580 | 550 |
| MaxJumps | 2 | 3 | 1 |
| EnergyRegenRate | 8 | 12 | 6 |
| EnergyCostPerAttack | 5 | 4 | 6 |

5. 展开 **LightAttacks** 数组，添加 3 个元素：
   - Light1: Damage=10, Startup=2, Active=3, Recovery=4, HitStop=4
   - Light2: Damage=12, Startup=2, Active=3, Recovery=4, HitStop=5
   - Light3: Damage=15, Startup=3, Active=4, Recovery=5, HitStop=6, bLaunchesEnemy=true, LaunchForce=400
6. **HeavyAttack**: Damage=25, Startup=5, Active=4, Recovery=8, HitStop=8, bLaunchesEnemy=true
7. **SpecialMove**: Damage=50, Startup=8, Active=6, Recovery=12, HitStop=12, bLaunchesEnemy=true
8. 重复创建 `DA_Tangtang` 和 `DA_Kiguemaru`

### 方法 B：使用 C++ Factory（快捷）

1. 在编辑器输出控制台运行：
```python
from first_game.data_assets.character_data_factory import UCharacterDataFactory
huikong_data = UCharacterDataFactory.CreateHuikongData(None)
```

---

## Step 3: 创建 Enhanced Input 资产

### 3.1 创建 InputAction

1. **Content Browser** → 右键 → **Input → Input Action**
2. 创建以下 InputAction：

| 名称 | ValueType |
|------|-----------|
| IA_Move | Axis2D (Vector2D) |
| IA_Jump | Digital (bool) |
| IA_LightAttack | Digital (bool) |
| IA_HeavyAttack | Digital (bool) |
| IA_Special | Digital (bool) |
| IA_Dodge | Digital (bool) |

### 3.2 创建 InputMappingContext

1. 右键 → **Input → Input Mapping Context**
2. 命名为 `IMC_Default`
3. 添加映射（Mappings）：

**键盘映射**:
| Action | Key |
|--------|-----|
| IA_Move | W/A/S/D + Arrow Keys |
| IA_Jump | Space |
| IA_LightAttack | J |
| IA_HeavyAttack | K |
| IA_Special | L |
| IA_Dodge | Left Shift |

**手柄映射**:
| Action | Key |
|--------|-----|
| IA_Move | Left Stick |
| IA_Jump | Gamepad Face Button Bottom (A) |
| IA_LightAttack | Gamepad Face Button Left (X) |
| IA_HeavyAttack | Gamepad Face Button Top (Y) |
| IA_Special | Gamepad Face Button Right (B) |
| IA_Dodge | Gamepad Left Shoulder (LB) |

---

## Step 4: 创建 BehaviorTree（敌人 AI）

1. **Content Browser** → 右键 → **Artificial Intelligence → Behavior Tree**
2. 命名为 `BT_EnemyAI`
3. 创建 Blackboard：右键 → **Artificial Intelligence → Blackboard** → `BB_Enemy`
4. 在 Blackboard 中添加 Key：
   - `PlayerLocation` (Vector)
   - `DistanceToPlayer` (Float)
   - `DetectionRange` (Float, default=600)
   - `AttackRange` (Float, default=80)

5. 在 Behavior Tree 中搭建：
```
Root (Selector)
├── Sequence: In Combat Range
│   ├── Condition: DistanceToPlayer < AttackRange
│   └── Task: BTTaskEnemyAttack
── Sequence: Chase Player
│   ├── Condition: DistanceToPlayer < DetectionRange
│   └── Task: BTTaskChasePlayer
└── Sequence: Detect Player
    └── Task: BTTaskDetectPlayer
```

---

## Step 5: 导入 Sprite 资产

运行导入脚本：
```bash
cd /home/claude/.frontend/first-game
python3 tools/import_sprites_to_ue5.py --content-path src_ue5/Content
```

然后在 UE5 编辑器中：
1. **Content Browser** → 找到 `Content/Characters/`
2. 右键 PNG 文件 → **Texture → Create AnimatedSprite**（Paper2D）
3. 设置 FPS：idle=8, walk=10, run=12, attack=15

**或者** 使用 UE5 Python API 自动导入：
```
Edit → Python → Execute Script → 选择 auto_import_sprites.py
```

---

## Step 6: 搭建测试 Level

### 6.1 创建 Level

1. **File → New Level** → 选择 `Basic` 模板
2. 命名为 `TestLevel_Combat`

### 6.2 放置玩家

1. **Place Actors** → 搜索 `PlayerCharacter`
2. 拖入场景，位置 (0, 0, 0)
3. Details 面板：
   - **CharacterData**: 选择 `DA_Huikong`
   - **DefaultMappingContext**: 选择 `IMC_Default`

### 6.3 放置敌人

1. 拖入 `BaseEnemy`，位置 (300, 0, 0)
2. Details：
   - **EnemyData**: 创建 `DA_EnemyGoblin`（复制角色 DataAsset 模板，降低数值）
   - **AIControllerClass**: `EnemyAIController`
   - **BehaviorTree**: `BT_EnemyAI`

### 6.4 放置地板

1. 拖入 `Box Brush` 或 `Plane`
2. Scale: X=10, Y=1, Z=0.5
3. 添加 Collision

### 6.5 放置摄像机

1. 拖入 `CameraActor`
2. 位置: (0, -800, 400)（侧视角度）
3. Rotation: (0, 0, 0)
4. **Post Process**: 设置 Orthographic Width = 960（半屏宽度）

### 6.6 设置 GameMode

1. **World Settings** → Game Mode Override
2. 选择 `FirstGameGameMode`
3. **Default Pawn Class**: `PlayerCharacter`

---

## Step 7: 创建 HUD Widget

1. **Content Browser** → 右键 → **User Interface → Widget Blueprint**
2. 命名为 `WBP_HUD`
3. 在 Designer 中添加：
   - **ProgressBar** × 2（HP Bar 绿色, Energy Bar 蓝色）
   - **TextBlock** × 2（HP 文字, Combo 文字）
   - 布局：HP 左上，Energy 左下，Combo 中上
4. 绑定变量名（Bind）：
   - HealthBar → `HealthBar`
   - HealthText → `HealthText`
   - EnergyBar → `EnergyBar`
   - ComboText → `ComboText`
5. 在 Event Graph 中：
   - Event Construct → Get SignalBus → 绑定 OnPlayerHealthChanged → UpdateHealth
   - 绑定 OnComboUpdated → UpdateCombo

---

## Step 8: 测试运行

1. **Play → Selected Viewport**（或 Standalone）
2. 测试项目：
   - [ ] WASD 移动角色
   - [ ] Space 跳跃
   - [ ] J 轻攻 → 敌人扣血
   - [ ] K 重攻 → HitStop 效果
   - [ ] L 必杀 → 高伤害 + 击飞
   - [ ] Shift 闪避
   - [ ] 连招：J → J → J → K（4连）
   - [ ] 敌人 AI：检测到玩家 → 追击 → 攻击
   - [ ] HUD：HP 条下降，Combo 数字显示

---

## 常见问题

| 问题 | 解决方案 |
|------|---------|
| 角色不动 | 检查 InputMappingContext 是否分配 |
| 攻击无伤害 | 检查 HitBox/HurtBox Collision Channel 设置 |
| HitStop 无效 | 检查 WorldSettings TimeDilation 是否被重置 |
| 敌人不追击 | 检查 BehaviorTree 是否分配到 AIController |
| 编译错误 | 检查 FirstGame.h PCH 包含 |
