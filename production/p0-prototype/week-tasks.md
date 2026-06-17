# P0 原型 — 每周任务清单

> **角色**: 气功小武僧 (Huikong)
> **周期**: 3 周 (Day 1-21)

---

## Week 1: 工具 + 风格 + Block-out

### Day 1 (安装)

- [ ] 安装 Blender 4.x
- [ ] 配置 Blender (English, Node Wrangler 插件)
- [ ] 注册 Mixamo 账户 (Adobe 账户)
- [ ] 注册 Meshy.ai 试用账号
- [ ] 通读本指南

### Day 2 (Blender 入门)

- [ ] 完成 Blender 界面导览 (30min)
- [ ] 学习基础操作: 移动/旋转/缩放 (G/R/S)
- [ ] 学习视图操作: 中键旋转/Shift 平移/滚轮缩放
- [ ] 练习: 创建简单几何体组合 (30min)
- [ ] 练习: 导入/导出 FBX (30min)

### Day 3 (Q版比例)

- [ ] 打开 `references/chibi-proportions.md`
- [ ] 在 Blender 中创建比例参考方块 (120cm 总高, 40cm 头)
- [ ] 调整摄像机到正交视图 (NumPad 5, NumPad 1)
- [ ] 练习: 在比例框架内搭建粗略人形
- [ ] 测试 FBX 导出

### Day 4 (Meshy 探索)

- [ ] 登录 Meshy.ai
- [ ] 用 prompt "chibi 3-head-tall cute martial arts monk, blue outfit, bald head" 生成 5 个草稿
- [ ] 下载最接近的 1 个，导入 Blender 分析
- [ ] 调整比例到准确三头身
- [ ] 整理武僧设计参考 (`references/huikong-design.md`)

### Day 5 (武僧建模 - 身体)

- [ ] 开始建模基础身体 (头/躯干/手臂/腿)
- [ ] 保持圆润轮廓 (Subdivision Surface modifier)
- [ ] 三头身比例检查
- [ ] T-Pose 姿势 (用于后续 Mixamo 绑定)
- [ ] 保存版本 (huikong_body_v01.blend)

### Day 6 (武僧建模 - 衣服)

- [ ] 建模基础僧衣 (覆盖身体)
- [ ] 飘带基础形状 (2 条)
- [ ] 护腕 + 腰带
- [ ] 检查衣服与身体的穿插
- [ ] 保存版本 (huikong_body_v02.blend)

### Day 7 (武僧建模 - 收尾)

- [ ] 细节: 念珠、额间红点、鞋
- [ ] 应用 Subdivision Surface
- [ ] 检查拓扑 (四边形为主, < 15k 面)
- [ ] T-Pose 最终检查
- [ ] **导出 FBX 测试** → 导入 UE5 检查尺寸
- [ ] 📝 Week 1 评审记录

**Week 1 交付物**:
- ✅ 武僧 block-out 模型 FBX
- ✅ 三头身比例正确
- ✅ UE5 导入测试通过

---

## Week 2: 贴图 + Mixamo 绑定 + 基础动画

### Day 8 (UV 展开)

- [ ] UV 标记 Seam (接缝)
- [ ] UV Unwrap
- [ ] 检查 UV 利用率和拉伸
- [ ] UV 布局整理

### Day 9 (贴图绘制)

- [ ] 创建材质槽位
- [ ] Base Color 贴图 (可手绘, 2048×2048)
- [ ] 配色参考: `references/huikong-design.md`
- [ ] Shadow Color 贴图 (可选, 也可在 UE5 材质中处理)
- [ ] 保存版本 (huikong_textured_v01.blend)

### Day 10 (细节贴图)

- [ ] 完善贴图细节
- [ ] 导出贴图 (PNG/TGA)
- [ ] FBX 导出设置:
  - 包含 Mesh + UV + Material
  - 不包含骨骼 (Mixamo 会加)
  - Apply Scale & Rotation
- [ ] 导出 huikong_final_mesh.fbx

### Day 11 (Mixamo 绑定)

- [ ] 登录 Mixamo.com
- [ ] Upload Character (上传 FBX)
- [ ] Auto-Rig:
  - 调整下巴位置
  - 调整手腕位置
  - 调整膝盖位置
  - 调整手肘位置
  - 确认所有绿色标记正确
- [ ] 完成绑定
- [ ] 下载 Rigged Character FBX

### Day 12 (Mixamo 动画 - 移动)

- [ ] 在 Mixamo 中搜索动画:
  - "idle" → 选择循环好的
  - "walk" → 选择自然的
  - "run" → 选择有活力的
  - "jump" → 选择高度合适的
- [ ] 每个动画调整参数 (速度/幅度)
- [ ] 下载 "Without Skin" 版本 (仅动画)
- [ ] 导入 Blender, 应用到绑定角色
- [ ] 预览效果

### Day 13 (动画微调)

- [ ] 检查 Q版比例下的动画效果:
  - 步幅是否合适 (腿短 → 步幅应小)
  - 手臂摆动是否协调
  - 跳跃高度是否合理
- [ ] Blender 中调整关键帧 (如需要)
- [ ] 确保所有动画循环无缝
- [ ] 导出 FBX (含骨骼 + 动画)

### Day 14 (UE5 集成测试)

- [ ] 云服务器恢复后上传:
  - Rigged Character FBX
  - 所有动画 FBX
  - Base Color 贴图
- [ ] UE5 导入 SkeletalMesh
- [ ] 创建 Skeleton + Physics Asset
- [ ] 导入动画, 测试 Retarget
- [ ] 测试基础动画播放
- [ ] 📝 Week 2 评审记录

**Week 2 交付物**:
- ✅ Mixamo 绑定完成
- ✅ 4 个基础动画 (idle/walk/run/jump)
- ✅ UE5 导入成功, 动画可播放

---

## Week 3: 战斗动画 + NPR + P0 评审

### Day 15 (轻攻动画)

- [ ] 手工制作 Light Attack 1 (左直拳)
- [ ] 手工制作 Light Attack 2 (右勾拳)
- [ ] 手工制作 Light Attack 3 (双掌推)
- [ ] 每招包含: 蓄力 + 出击 + 收招
- [ ] 确保连招衔接流畅

### Day 16 (重攻 + 技能)

- [ ] 手工制作 Heavy Attack (气功波蓄力推出)
- [ ] 手工制作 Skill 1 (气功波远程)
- [ ] 测试 Animation Notify 触发点

### Day 17 (闪避 + 受击)

- [ ] 手工制作 Dodge (气功瞬移效果)
- [ ] 从 Mixamo 选 Hit Reaction 并修改
- [ ] Death 动画 (可从 Mixamo 修改)

### Day 18 (UE5 Animation Blueprint)

- [ ] 创建 Animation Blueprint (ABP_Huikong)
- [ ] 实现 State Machine:
  - Locomotion (Idle/Walk/Run blend)
  - Jump (Jump → Fall → Land)
  - Attack (Light1 → Light2 → Light3)
  - Dodge
  - Hit
- [ ] 使用 BlendSpace 实现移动混合

### Day 19 (UE5 NPR 材质)

- [ ] 参考 `references/npr-reference.md`
- [ ] 创建 Material: M_Huikong_Toon
- [ ] 实现 2-tone shading
- [ ] 实现 Rim Light
- [ ] 实现描边 (Inverted Hull 方法)
- [ ] 应用到武僧模型, 测试效果

### Day 20 (整合测试)

- [ ] 在 UE5 中测试:
  - 移动 (WASD) + 动画混合
  - 攻击 (J/K) + 动画播放
  - 闪避 (Space) + 动画播放
  - 锁定 (Tab) + 相机切换
  - NPR 渲染效果
- [ ] 修复发现的问题
- [ ] 性能检查 (> 60fps)

### Day 21 (P0 评审)

- [ ] 准备 P0 评审文档
- [ ] 录制演示视频 (30s)
- [ ] 总结:
  - 风格是否满意
  - 手感是否达标
  - 管线是否可行
  - 后续调整方向
- [ ] 📝 P0 评审报告

**Week 3 交付物**:
- ✅ 完整战斗动画集 (15 个动画)
- ✅ UE5 Animation Blueprint
- ✅ NPR 卡通渲染材质
- ✅ P0 可玩原型 (移动 + 攻击)
- ✅ P0 评审报告

---

## 📝 评审模板

### Week 1 评审 (Day 7)

```
## Week 1 评审

### 完成项
- [ ] 武僧 block-out 完成
- [ ] 三头身比例准确

### 风格评估
- 轮廓圆润度: 1-5 分
- 职业识别度: 1-5 分
- Q版 可爱度: 1-5 分

### 问题 & 调整
- 问题 1: ...
- 问题 2: ...

### Week 2 计划调整
- ...
```

### P0 最终评审 (Day 21)

```
## P0 最终评审

### 核心假设验证
- [ ] Q版 3D 角色的连招战斗是否足够爽?
- [ ] NPR 卡通渲染风格是否讨喜?
- [ ] 打击感是否达到基本要求?

### 评分 (1-5)
- 角色模型质量: _
- 动画流畅度: _
- 战斗手感: _
- 视觉风格: _
- 性能表现: _

### 是否继续方案 E?
- [ ] 是, 继续当前方向
- [ ] 需调整, 具体问题: ...
- [ ] 放弃, 改方案: ...

### 下一步建议
- ...
```

---

## ⏱️ 每日时间分配建议

| 活动 | 时长 | 备注 |
|---|---|---|
| Blender 建模/动画 | 3-4h | 核心工作 |
| 学习/查资料 | 1h | 遇到问题时 |
| Mixamo 操作 | 0.5h | 动画选择/下载 |
| UE5 测试 | 0.5h | 每日集成测试 |
| 记录/评审 | 0.5h | 每日小结 |
| **总计** | **5-6h** | 按半天工作日 |
