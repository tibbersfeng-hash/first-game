# Huikong 3D 资产导入指南

## 资产清单

### 基础模型
- **文件**: `model_3.fbx` (33MB)
- **位置**: `design/assets/output/3d/huikong/model_3.fbx`
- **说明**: 慧空角色的 T-Pose 基础模型

### 动画文件 (7个)
1. **待机-1** (Idle_01) - 基础待机动画
2. **待机-2** (Idle_02) - 备选待机动画
3. **走路-1** (Walk_01) - 行走动画
4. **奔跑** (Run_01) - 奔跑动画
5. **回旋踢** (SpinKick) - 轻攻击动画
6. **受击** (HitReaction) - 受击反馈动画
7. **落地** (Landing) - 跳跃落地动画

## UE5 导入步骤

### 1. 同步资产到远程服务器

```bash
cd /home/claude/.frontend/first-game
rsync -avz design/assets/output/3d/huikong/ gpu:/root/autodl-tmp/project/first-game/design/assets/output/3d/huikong/
```

### 2. 在 UE5 中创建目录结构

在 Content Browser 中创建：
```
/Content/Characters/Huikong/
├── Mesh/           # 网格体
├── Skeleton/       # 骨骼
├── Physics/        # 物理资产
├── Animations/     # 动画
├── ABP/           # Animation Blueprint
└── Materials/     # 材质
```

### 3. 导入基础模型

**FBX 导入设置**：
- ✅ Import Mesh
- ✅ Skeleton: (Create New) → `SK_Huikong`
- ✅ Import Materials: ❌ (后续手动创建 NPR 材质)
- ✅ Import Animations: ❌ (动画单独导入)
- ✅ Uniform Scale: 1.0
- ✅ Force Front XAxis: ✅
- ✅ Convert Scene: ✅ (Y-up to Z-up)

**导入后检查**：
- 模型尺寸是否正确（约 120cm 高度）
- 法线方向是否正确
- 网格体是否完整

### 4. 导入动画

对每个动画文件使用以下设置：
- ✅ Import Animations
- ✅ Skeleton: `SK_Huikong` (复用已有骨骼)
- ❌ Import Mesh: (取消勾选)
- ✅ Animation: (Create New)
- ✅ Uniform Scale: 1.0

**动画重命名**：
- `待机-1` → `Anim_Huikong_Idle_01`
- `待机-2` → `Anim_Huikong_Idle_02`
- `走路-1` → `Anim_Huikong_Walk_01`
- `奔跑` → `Anim_Huikong_Run_01`
- `回旋踢` → `Anim_Huikong_LightAttack_01`
- `受击` → `Anim_Huikong_HitReaction`
- `落地` → `Anim_Huikong_Landing`

### 5. 创建 Physics Asset

在 Skeleton Editor 中：
1. 打开 `SK_Huikong`
2. 菜单: `Asset Actions` → `Create Physics Asset`
3. 使用默认设置（Capsule 类型）
4. 保存为 `PA_Huikong`

### 6. 测试动画

在 Skeleton Editor 中：
1. 切换到 `Asset Viewport`
2. 从 Content Browser 拖拽动画到视口
3. 检查动画播放是否正常
4. 检查是否有穿模或异常

## 常见问题

### Q: 模型尺寸不对？
**A**: 检查 FBX 导入时的 Scale 设置，可能需要调整为 100 或 0.01

### Q: 动画骨骼不匹配？
**A**: 确保所有动画使用同一个 Skeleton (`SK_Huikong`)

### Q: 模型朝向错误？
A**: 导入时勾选 `Force Front XAxis` 和 `Convert Scene`

## 下一步

导入完成后，继续：
1. **创建 Animation Blueprint** (优先级 2)
2. **实现 NPR 材质** (优先级 3)
3. **集成到 PlayerCharacter** (优先级 4)
