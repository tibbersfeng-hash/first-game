# GUI 操作详细指南 — 格斗萌主 NPR 材质与动画配置

> **创建日期**: 2026-06-21
> **目标**: 通过 Editor GUI 完成 NPR 材质、描边、AnimBP、LOD 配置
> **预计时间**: 3-4 小时

---

## 📋 前置准备

### 1. 连接到编辑器

**方式 A: VNC 直接连接 (推荐)**
```bash
# 本地电脑执行
ssh -L 5901:localhost:5901 vipuser@<服务器 IP>

# 然后用 VNC 客户端连接
vnc://localhost:5901
密码：(你的 VNC 密码)
```

**方式 B: Parsec 串流 (低延迟)**
```bash
# 服务器已安装 Parsec，直接连接
# 参考：tools/server-init-parsec.sh
```

**方式 C: MCP 远程操控 (有限支持)**
```bash
# MCP 可执行简单操作，但材质编辑需 GUI
# 仅用于截图验证
```

### 2. 确认编辑器状态

打开编辑器后检查：
- [ ] 内容浏览器可见 (`Content/`)
- [ ] 输出日志窗口可见 (`Window → Developer Tools → Output Log`)
- [ ] 材质编辑器可打开 (双击任意材质)

---

##  第一部分：NPR 父材质创建 (M_NPR_Parent)

### 步骤 1: 创建材质资产

1. 在 **Content Browser** 中导航到 `Content/Materials/`
   - 如果目录不存在：右键 `Content` → `New Folder` → 命名 `Materials`

2. 右键空白处 → `Material` → 命名 `M_NPR_Parent`

3. 双击打开材质编辑器

### 步骤 2: 设置材质属性

在 **Details** 面板 (右侧) 设置：

| 属性 | 值 | 位置 |
|------|-----|------|
| **Shading Model** | Unlit | Material → Shading Model |
| **Blend Mode** | Opaque | Material → Blend Mode |
| **Two Sided** | No | Material → Two Sided |
| **Used With Skeletal Mesh** | ✅ | Material → Usage |
| **Used With Static Lighting** | ❌ | Material → Usage |

### 步骤 3: 添加参数节点

在材质图表中，右键搜索添加以下节点：

#### 3.1 BaseColor 参数
1. 右键 → `Vector Parameter` → 命名 `BaseColor`
2. 设置 **Default Value**: 
   - R: 0.8, G: 0.6, B: 0.4, A: 1.0
   - (小麦肤色)

#### 3.2 ShadowColor 参数
1. 右键 → `Vector Parameter` → 命名 `ShadowColor`
2. 设置 **Default Value**:
   - R: 0.3, G: 0.15, B: 0.2, A: 1.0
   - (深紫阴影)

#### 3.3 RimColor 参数
1. 右键 → `Vector Parameter` → 命名 `RimColor`
2. 设置 **Default Value**:
   - R: 1.0, G: 0.9, B: 0.7, A: 1.0
   - (暖白 Rim)

#### 3.4 ShadowThreshold 参数
1. 右键 → `Scalar Parameter` → 命名 `ShadowThreshold`
2. 设置 **Default Value**: 0.3

#### 3.5 LightDirection 常量
1. 右键 → `Constant3Vector` → 命名 `LightDirection`
2. 设置 **Constant**:
   - R: 0.5, G: 0.3, B: 0.8
   - (归一化光方向)

### 步骤 4: 构建 2-tone 阴影逻辑

#### 4.1 添加 VertexNormalWS
1. 右键 → `VertexNormalWS`

#### 4.2 计算 NdotL
1. 右键 → `DotProduct` → 命名 `NdotL`
2. 连接：
   - **A**: VertexNormalWS
   - **B**: LightDirection

#### 4.3 阴影阈值判断
1. 右键 → `Step` → 命名 `ShadowStep`
2. 连接：
   - **A** (Edge): ShadowThreshold
   - **B** (Value): NdotL
3. 输出：0 = 阴影，1 = 受光

#### 4.4 颜色混合
1. 右键 → `Lerp` → 命名 `ColorLerp`
2. 连接：
   - **A**: ShadowColor (阴影色)
   - **B**: BaseColor (基础色)
   - **Alpha**: ShadowStep

### 步骤 5: 添加 Rim Light

#### 5.1 简化版 Rim (固定强度)
1. 右键 → `Constant` → 命名 `RimScale`
2. 设置 **R**: 0.3

#### 5.2 Rim 贡献
1. 右键 → `Multiply` → 命名 `RimContrib`
2. 连接：
   - **A**: RimColor
   - **B**: RimScale

### 步骤 6: 合成最终颜色

1. 右键 → `Add` → 命名 `FinalColor`
2. 连接：
   - **A**: ColorLerp (2-tone 颜色)
   - **B**: RimContrib (Rim 光)

3. 将 **FinalColor** 的 RGB 输出连接到 **Emissive Color**

### 步骤 7: 保存并编译

1. 点击 **Apply** (左上角)
2. 点击 **Save** (Ctrl+S)
3. 等待编译完成 (底部进度条)

### 验证截图

材质图应该类似：
```
[BaseColor] ──┐
              ├─ [Lerp] ──
[ShadowColor]─┘           │
              ↑           ├─ [Add] ── [Emissive Color]
[ShadowThreshold]─┐       │
                  │       │
[LightDirection]─┐│       │
                 ││       │
[VertexNormalWS]─┴┴─[Dot]─[Step]
                           ↑
[RimColor] ────────┐      │
                   ├─[Mul]┘
[RimScale: 0.3] ───┘
```

---

## 🎨 第二部分：4 怪物材质实例创建

### 步骤 1: 创建材质实例

对每只怪物重复以下步骤：

#### CandyZombie (绿色僵尸)
1. 在 Content Browser 中右键 `M_NPR_Parent`
2. 选择 `Create Material Instance`
3. 命名：`MI_CandyZombie_NPR`
4. 双击打开

5. 在 **Parameters** 区域设置：
   - **BaseColor**: R:0.55, G:0.85, B:0.60 (青绿)
   - **ShadowColor**: R:0.25, G:0.15, B:0.30 (腐烂紫)
   - **RimColor**: R:0.80, G:1.00, B:0.70 (淡绿)
   - **ShadowThreshold**: 0.3 (保持默认)

6. Save (Ctrl+S)

#### Gingerbread (棕色姜饼人)
1. 创建材质实例：`MI_Gingerbread_NPR`
2. 设置参数：
   - **BaseColor**: R:0.85, G:0.55, B:0.30 (暖棕)
   - **ShadowColor**: R:0.35, G:0.12, B:0.08 (深红棕)
   - **RimColor**: R:1.00, G:0.85, B:0.40 (金黄)

#### ShadowNinja (紫色忍者)
1. 创建材质实例：`MI_ShadowNinja_NPR`
2. 设置参数：
   - **BaseColor**: R:0.40, G:0.20, B:0.55 (深紫)
   - **ShadowColor**: R:0.15, G:0.05, B:0.20 (品红阴影)
   - **RimColor**: R:0.30, G:0.70, B:1.00 (电光蓝)

#### ArmoredGum (银白装甲)
1. 创建材质实例：`MI_ArmoredGum_NPR`
2. 设置参数：
   - **BaseColor**: R:0.90, G:0.92, B:0.95 (银白)
   - **ShadowColor**: R:0.15, G:0.20, B:0.35 (深蓝)
   - **RimColor**: R:1.00, G:1.00, B:1.00 (纯白)

### 步骤 2: 应用材质到怪物

对每只怪物：

1. 导航到 `Content/Monsters/<怪物名>/Mesh/`
2. 双击打开 `SK_<怪物名>` (SkeletalMesh)
3. 在 **Details** 面板找到 **Materials** 数组
4. 展开 **Element 0**
5. 点击材质槽，选择对应的 `MI_<怪物名>_NPR`
6. Save (Ctrl+S)

**或者** 通过蓝图应用：

1. 导航到 `Content/Monsters/<怪物名>/Blueprint/`
2. 双击打开 `BP_<怪物名>`
3. 在 **Viewport** 中选中 SkeletalMeshComponent
4. 在 **Details** → **Materials** → **Element 0** 设置材质实例
5. Compile & Save

---

##  第三部分：描边材质 (Inverted Hull)

### 步骤 1: 创建描边父材质

1. 在 `Content/Materials/` 中创建新材质：`M_Outline`
2. 双击打开

### 步骤 2: 设置材质属性

| 属性 | 值 |
|------|-----|
| **Shading Model** | Unlit |
| **Blend Mode** | Masked |
| **Two Sided** | ✅ Yes |
| **Used With Skeletal Mesh** | ✅ |

### 步骤 3: 添加参数

1. **OutlineColor** (Vector Parameter)
   - Default: R:0, G:0, B:0 (黑色)

2. **OutlineWidth** (Scalar Parameter)
   - Default: 2.0 (单位：cm)

### 步骤 4: 构建描边逻辑

#### 4.1 背面检测
1. 添加 `VertexNormalWS`
2. 添加 `CameraVectorWS`
3. 添加 `DotProduct` → 命名 `NdotV`
   - A: VertexNormalWS
   - B: CameraVectorWS

4. 添加 `Constant` → 命名 `Zero`
   - R: 0.0

5. 添加 `If` → 命名 `BackFaceMask`
   - A: NdotV
   - B: Zero
   - A > B: 0 (正面隐藏)
   - A = B: 0
   - A < B: 1 (背面显示)

6. 将 **BackFaceMask** 输出连接到 **Opacity Mask**

#### 4.2 顶点挤出 (WPO)
1. 添加 `Multiply` → 命名 `WPO`
   - A: VertexNormalWS
   - B: OutlineWidth

2. 将 **WPO** 输出连接到 **World Position Offset**

#### 4.3 描边颜色
1. 将 **OutlineColor** 直接连接到 **Emissive Color**

### 步骤 5: 保存并编译

### 步骤 6: 创建描边 Actor (蓝图方式)

由于描边需要单独的 SkeletalMeshComponent，创建蓝图：

1. 在 `Content/Monsters/<怪物名>/Blueprint/` 中创建新蓝图
2. 命名：`BP_Outline_<怪物名>`
3. 父类：`Actor`

4. 添加组件：
   - **SkeletalMeshComponent** → 命名 `OutlineMesh`
   - 设置 Mesh 为怪物的 SkeletalMesh
   - 设置 Material 为 `M_Outline` 的材质实例

5. 在 **Event Graph** 中：
   - **Event BeginPlay** → **AttachToComponent** (Attach 到父 Actor)

6. Compile & Save

### 步骤 7: 为每怪物创建描边材质实例

1. 右键 `M_Outline` → `Create Material Instance`
2. 命名：`MI_Outline_<怪物名>`
3. 设置参数：
   - **OutlineColor**: (0, 0, 0) 黑色
   - **OutlineWidth**:
     - 武僧：2.0
     - 魔导师：1.5 (纤细)
     - 狂战士：3.0 (粗犷)
     - 4 怪物：2.0 (标准)

---

## 🎬 第四部分：AnimBP 状态机配置

### 步骤 1: 打开 AnimBP

1. 导航到 `Content/Characters/Huikong/ABP/`
2. 双击 `ABP_Huikong`

### 步骤 2: 创建状态机

1. 在 **AnimGraph** 中右键 → `Add State Machine` → 命名 `MovementSM`

2. 双击进入状态机

### 步骤 3: 添加状态

添加以下状态：
- **Idle** (默认状态)
- **Walk**
- **Run**
- **Attack**
- **HitStun**
- **Death**

### 步骤 4: 配置过渡规则

#### Idle → Walk
- **规则**: `Speed > 10`
- **Blend Time**: 0.2s

#### Walk → Run
- **规则**: `Speed > 300`
- **Blend Time**: 0.2s

#### Run → Walk
- **规则**: `Speed < 300`
- **Blend Time**: 0.2s

#### Walk → Idle
- **规则**: `Speed < 10`
- **Blend Time**: 0.2s

#### Any State → Attack
- **规则**: `IsAttacking == true`
- **Blend Time**: 0.1s

#### Attack → Idle
- **规则**: `AttackFinished == true`
- **Blend Time**: 0.2s

#### Any State → HitStun
- **规则**: `IsHit == true`
- **Blend Time**: 0.05s

#### HitStun → Idle
- **规则**: `HitStunTimer <= 0`
- **Blend Time**: 0.2s

#### Any State → Death
- **规则**: `Health <= 0`
- **Blend Time**: 0.3s

### 步骤 5: 连接动画

对每个状态：
1. 双击进入状态
2. 添加 `Play Animation` 节点
3. 选择对应动画：
   - Idle: `AM_Huikong_Idle_01`
   - Walk: `AM_Huikong_Walk_01`
   - Run: `AM_Huikong_Run_01`
   - Attack: `AM_Huikong_LightAttack_01`
   - HitStun: `AM_Huikong_HitReaction`
   - Death: (创建或导入死亡动画)

### 步骤 6: 添加 BlendSpace (可选)

为 Walk/Run 创建 BlendSpace：
1. 右键 `Content/Characters/Huikong/Animations/` → `BlendSpace1D`
2. 命名：`BS_Locomotion`
3. 添加动画：
   - Position 0: `AM_Huikong_Idle_01`
   - Position 100: `AM_Huikong_Walk_01`
   - Position 600: `AM_Huikong_Run_01`
4. 在状态机中使用 BlendSpace 替代单独动画

### 步骤 7: 编译并测试

1. Compile (Ctrl+Shift+B)
2. Save (Ctrl+S)
3. 在 **Preview Scene Settings** 中测试动画切换

---

## 📊 第五部分：LOD 配置

### 步骤 1: 准备 LOD 模型

确认 FBX 文件存在：
- `Content/Characters/Huikong/Mesh/model_3.fbx` (LOD0, 40k 面)
- `Content/Characters/Huikong/Mesh/model_3_LOD1.fbx` (LOD1, 10k 面)
- `Content/Characters/Huikong/Mesh/model_3_LOD2.fbx` (LOD2, 2.5k 面)

### 步骤 2: 配置 SkeletalMesh LOD

1. 双击打开 `SKM_Huikong`
2. 在 **Details** 面板找到 **LOD Settings**
3. 展开 **LOD Groups**

#### LOD 0 (最高细节)
- **Screen Size**: 1.0 (始终显示)
- **Mesh**: 使用当前高模

#### LOD 1 (中等细节)
- **Screen Size**: 0.3 (距离约 10m)
- **Mesh**: 导入 LOD1 FBX
  - 点击 **Import** → 选择 `model_3_LOD1.fbx`
  - 设置 **Import Type**: Skeletal Mesh
  - 勾选 **Update Skeleton Reference Pose**

#### LOD 2 (最低细节)
- **Screen Size**: 0.1 (距离约 20m)
- **Mesh**: 导入 LOD2 FBX

### 步骤 3: 设置 LOD 过渡

1. 在 **LOD Settings** 中找到 **LOD Transition**
2. 设置：
   - **LOD Transition Type**: Automatic
   - **LOD Cross-Fade Time**: 0.1s

### 步骤 4: 为怪物配置 LOD

重复步骤 2-3，为每只怪物配置：
- `SK_CandyZombie`
- `SK_Gingerbread`
- `SK_ShadowNinja`
- `SK_ArmoredGum`

### 步骤 5: 验证 LOD 切换

1. 在编辑器中放置怪物 Actor
2. 移动相机远离/靠近
3. 观察 LOD 切换是否平滑
4. 使用 **Stat Unit** 命令查看帧率变化

---

## ✅ 验收清单

### NPR 材质
- [ ] M_NPR_Parent 材质创建成功
- [ ] 材质属性：Unlit, Opaque, SkeletalMesh
- [ ] 4 怪物材质实例创建
- [ ] 材质参数正确 (BaseColor, ShadowColor, RimColor)
- [ ] 材质应用到怪物模型
- [ ] 截图验证：2-tone 阴影可见
- [ ] 截图验证：Rim Light 可见

### 描边材质
- [ ] M_Outline 材质创建成功
- [ ] 材质属性：Unlit, Masked, TwoSided
- [ ] 描边逻辑：背面检测 + WPO 挤出
- [ ] 4 怪物描边材质实例创建
- [ ] 描边 Actor 创建并 Attach
- [ ] 截图验证：黑色轮廓可见

### AnimBP 状态机
- [ ] 状态机创建 (Idle/Walk/Run/Attack/HitStun/Death)
- [ ] 过渡规则配置
- [ ] 动画连接正确
- [ ] BlendSpace 创建 (可选)
- [ ] 编译无错误
- [ ] 预览测试通过

### LOD 配置
- [ ] LOD0/LOD1/LOD2 模型导入
- [ ] Screen Size 设置正确
- [ ] LOD 过渡平滑
- [ ] 4 怪物 LOD 配置完成
- [ ] 帧率验证通过

---

## 📸 截图要求

完成后请截图以下场景：

1. **NPR 材质验证**
   - 4 怪物并排站立
   - 相机位置：前方 3m，高度 1.5m
   - 光照：DirectionalLight (Intensity: 3.0)

2. **描边验证**
   - 单怪物特写
   - 相机位置：前方 2m，高度 1m
   - 可见黑色轮廓

3. **动画验证**
   - 角色从 Idle → Walk → Run 过渡
   - 相机位置：第三人称越肩

4. **LOD 验证**
   - 同一怪物在不同距离的截图
   - 距离：1m, 10m, 20m

---

##  常见问题

### Q: 材质编译失败
**A**: 检查 Shading Model 是否为 Unlit，确认没有使用 PBR 节点

### Q: 描边不可见
**A**: 检查 Blend Mode 是否为 Masked，Two Sided 是否启用

### Q: 动画不切换
**A**: 检查过渡规则的 Bool 变量是否在蓝图中正确设置

### Q: LOD 切换闪烁
**A**: 增加 LOD Cross-Fade Time 到 0.2s

---

*指南创建：Claude Code | 2026-06-21*
