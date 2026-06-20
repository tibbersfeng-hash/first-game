# Session State — 格斗萌主

*Last updated: 2026-06-20 23:30*

## Current Phase
**P0 原型 — 4 怪物 UE5 导入完成 ✅，待可视化验证**

## 服务器信息
- **本机 GPU**: NVIDIA RTX 3090 (24GB)
- **UE5**: 5.7.4
- **项目路径**: `/home/vipuser/first-game/src_ue5/`
- **Blender**: 4.0.2 (本机 `/usr/bin/blender`)

## ✅ 已完成

### 代码系统 (48+ C++ 文件, 编译通过)
- [x] PlayerCharacter + BaseEnemy (GAS 集成)
- [x] 第三人称相机系统 (ADR-008)
- [x] Lock-On 目标锁定 (ADR-009)
- [x] ComboManager 连招系统 (ADR-007)
- [x] HitBox/HurtBox 碰撞检测
- [x] HitStopManager (打击顿帧) + HitCameraShake
- [x] CharacterStatsComponent 数值成长
- [x] DungeonFlow + DungeonRoom + WaveManager
- [x] DoorActor 通关门 + LootItem 战利品
- [x] HUDWidget + DamageNumberWidget + DeathWidget
- [x] EnemyAIController + BehaviorTree
- [x] LevelBuilder 程序化关卡
- [x] SignalBus 事件系统
- [x] AnimTestActor 动画测试

### 武僧 3D 资产
- [x] AI 3D 生成 (150k faces → 重拓扑 40k faces)
- [x] 混元绑骨 (7 个预设动作)
- [x] FBX + 贴图上传服务器
- [x] 动画 FBX: Idle×2, Walk, Run, LightAttack, HitReaction, Landing
- [x] ABP_Huikong 动画蓝图已创建 (GPU 服务器操作)
- [x] AM_Huikong_* AnimSequence 已导入 (GPU 服务器操作)

### 糖果地牢 4 怪物 3D 资产 (2026-06-20)
- [x] 混元3D AI 生成 4 怪物模型
- [x] 混元绑骨 (每怪物 6 个动作: Idle/Walk/Attack/Hit/Death + 1)
- [x] Blender 批量后处理 (`tools/blender_batch_fix_monsters.py`)
  - 顶点合并 (~10k verts removed/monster)
  - 法线重算 (bmesh fallback)
  - 骨骼验证 (28 bones/monster)
  - LOD 生成 (LOD1: 50%, LOD2: 25%)
- [x] UE5-ready FBX 导出 (36 files: 4 mesh + 8 LOD + 24 anim)
- [x] UE5 导入 (4 SkeletalMesh + 24 AnimSequence + 4 Material + 4 AnimBP = 40 uassets)
- 输出: `Content/Monsters/{CandyZombie,Gingerbread,ShadowNinja,ArmoredGum}/`
- 导入管线: `tools/import_monsters.sh` (Step 1=FBX, Step 2=Material+AnimBP)

### 测试框架
- [x] FirstGameTests 模块 (17 个测试文件, 72+ 断言)
- [x] 覆盖: 战斗/连招/碰撞/锁定/数值/信号/地牢/音频

### Build 系统清理 (2026-06-19)
- [x] 移除未使用的 Paper2D 依赖
- [x] 移除未使用的 GameplayCameras 依赖
- [x] 清理重复的 Private 依赖
- [x] FirstGameTests 添加到 Editor Target

### 资产导入方案
- [x] 编写 `import_skeletal_mesh.py` (骨骼网格体+贴图导入)
- [x] 编写 `run_headless_import.sh` (一键导入管线)
- [x] 文档 `docs/headless-asset-import.md`

## ❌ 阻塞问题

### 1. GPU 服务器不可用
- 状态: SSH 连接被拒 (端口 42491)
- 原因: 云 GPU 实例可能已停止/过期
- 影响: 无法运行 UE5 Editor/Cmd
- **行动**: 需要联系平台方恢复实例或申请新实例

### 2. Vulkan Swapchain 失败 (阻塞 UE5 Editor GUI)
- 错误: `vkGetPhysicalDeviceSurfacePresentModesKHR failed, VkResult=-13`
- 根因: 容器环境无 DRM/KMS 访问

#### 新方案 (待验证)
Editor 模式 + `-RunPythonScript` + `-nullrhi` + `-unattended`
- 之前在 Game 模式下崩溃 (GIsEditor=false)
- Editor 模式下 GIsEditor=true，断言应通过
- `-nullrhi` 跳过 Vulkan 渲染
- 详见 `docs/headless-asset-import.md`

#### 备选方案
1. VirtualGL + TurboVNC (3D 加速虚拟显示)
2. UE5 `-buildmachine` 模式
3. 自定义 C++ UCommandlet

## 📋 P0 原型进度

| 周次 | 主题 | 状态 |
|---|---|---|
| Week 1 | AI 3D 生成 + Blender 修整 | ✅ 完成 |
| Week 2 | 贴图 + 绑骨 + 动画 | ✅ 完成 (服务器端) |
| Week 3 | 战斗动画 + NPR + 整合 | ⏳ 待服务器恢复 |

### Week 3 待办
- [ ] 验证骨骼网格体导入
- [ ] 验证动画播放
- [ ] 创建/完善 Animation Blueprint
- [ ] 实现 NPR 卡通渲染材质
- [ ] 实现描边 (Inverted Hull)
- [ ] 整合测试 (移动+攻击+锁定)
- [ ] P0 评审

## 🔧 恢复后的操作步骤

```bash
# 1. 连接 GPU 服务器
ssh -L 5901:localhost:5901 -p <PORT> root@connect.nmb2.seetacloud.com

# 2. 拉取最新代码
cd /root/autodl-tmp/project/first-game && git pull

# 3. 编译
cd src_ue5 && make FirstGameEditor

# 4. 运行资产导入
bash /root/autodl-tmp/project/first-game/tools/run_headless_import.sh all

# 5. 验证
bash /root/autodl-tmp/project/first-game/tools/run_headless_import.sh test
```

## 📊 Git 状态
- 最新 commit: `7ec0f3d chore: clean Build.cs deps + add FirstGameTests to Editor target`
- 本地落后: 部分二进制资产 (FBX/uasset) 需通过 git fetch 或 scp 同步
- 同步方式: `git fetch origin --filter=blob:limit=100k` (跳过大型二进制)
