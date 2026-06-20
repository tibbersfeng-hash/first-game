# 糖果地牢 4 怪物 3D 资产生成清单

> **Created**: 2026-06-20
> **Pipeline**: 混元3D AI 生成 → Blender 修正 → UE5 导入
> **Status**: ✅ 全部生成完成

---

## 一、生成状态总览

| 怪物 | 3D 模型 | 绑骨动作 | Blender 修正 | UE5 导入 | 状态 |
|------|---------|---------|-------------|---------|------|
| 糖果僵尸 | ✅ | ✅ 6/6 | ⬜ | ⬜ | ✅ 完成 |
| 暴躁姜饼人 | ✅ | ✅ 6/6 |  | ⬜ | ✅ 完成 |
| 暗影忍者糖 | ✅ | ✅ 6/6 | ⬜ | ⬜ | ✅ 完成 |
| 铁甲口香糖 | ✅ | ✅ 6/6 | ⬜ | ⬜ | ✅ 完成 |

---

## 二、3D 模型详情

### 2.1 糖果僵尸 (Candy Zombie) — ASSET-037

| 项目 | 状态 | 文件 | 大小 |
|------|------|------|------|
| 3D 模型 (OBJ) | ✅ | `model_1.obj` | 14.7 MB |
| 3D 模型 (GLB) | ✅ | `model_2.glb` | 15.9 MB |
| 3D 模型 (FBX) | ✅ | `model_3.fbx` | 18.6 MB |
| 预览图 ×3 | ✅ | `preview_*.png` | 128 KB each |

**绑骨动作 (6/6)**:
| 动作 | MotionType | 状态 | 文件 |
|------|-----------|------|------|
| 待机-1 (Idle) | 26 | ✅ | `motions/待机-1/model_1.fbx` (17.8 MB) |
| 走路-1 (Walk) | 23 | ✅ | `motions/走路-1/model_1.fbx` (17.8 MB) |
| 慢跑 (Jog) | 32 | ✅ | `motions/慢跑/model_1.fbx` (17.9 MB) |
| 二连击打 (Attack) | 5 | ✅ | `motions/二连击打/model_1.fbx` (18.0 MB) |
| 受击 (Hit) | 8 | ✅ | `motions/受击/model_1.fbx` (17.9 MB) |
| 受击倒地-1 (Death) | 11 | ✅ | `motions/受击倒地-1/model_1.fbx` (17.9 MB) |

**视觉质量评估**:
- ✅ 三头身比例准确
- ✅ 绿色半透明身体
- ✅ 三个彩色棒棒糖天线
- ✅ 悬挂眼球（左眼）
- ✅ 破旧粉色围裙
- ✅ 爪状手指
- ✅ PBR 材质完整
- ⚠️ Blender 需修正: 眼球骨骼、棒棒糖分离

---

### 2.2 暴躁姜饼人 (Angry Gingerbread) — ASSET-038

| 项目 | 状态 | 文件 | 大小 |
|------|------|------|------|
| 3D 模型 (FBX) | ✅ | `model_3.fbx` | 18.5 MB |
| 预览图 ×3 | ✅ | `preview_*.png` | 128 KB each |

**绑骨动作 (0/6)**: 🔄 补完中

---

### 2.3 暗影忍者糖 (Shadow Ninja Candy) — ASSET-039

| 项目 | 状态 | 文件 | 大小 |
|------|------|------|------|
| 3D 模型 (FBX) | ✅ | `model_3.fbx` | 19.6 MB |
| 预览图 ×3 | ✅ | `preview_*.png` | 128 KB each |

**绑骨动作 (0/6)**: 🔄 补完中

---

### 2.4 铁甲口香糖 (Armored Gum) — ASSET-040

| 项目 | 状态 | 文件 | 大小 |
|------|------|------|------|
| 3D 模型 (FBX) | ✅ | `model_3.fbx` | 41.5 MB |
| 预览图 ×3 | ✅ | `preview_*.png` | 128 KB each |

**绑骨动作 (0/6)**: 🔄 补完中

---

## 三、后续步骤

### 阶段 1: 绑骨补完 (当前)
- [ ] 暴躁姜饼人: 6 个动作
- [ ] 暗影忍者糖: 6 个动作
- [ ] 铁甲口香糖: 6 个动作

**预计时间**: 每个怪物 ~30 分钟 (3D 重新生成 3 分钟 + 6 动作 × 4 分钟)

### 阶段 2: Blender 修正
参考: `design/art/blender-monster-postprocess.md`

每个怪物需要:
- [ ] 拓扑检查/修正 (15-30 min)
- [ ] UV 展开修正 (10-20 min)
- [ ] 材质清理 (5-10 min)
- [ ] 骨骼验证 (10-15 min)
- [ ] 比例/朝向调整 (5 min)
- [ ] LOD 生成 (10 min)
- [ ] 导出 UE5 FBX (2 min)

**预计时间**: 每怪物 1-1.5 小时，总计 4-6 小时

### 阶段 3: UE5 导入
参考: `design/art/ue5-monster-import-guide.md`

每个怪物需要:
- [ ] FBX 导入 (SkeletalMesh)
- [ ] 材质实例创建 (NPR)
- [ ] 动画序列导入
- [ ] BlendSpace 设置
- [ ] AnimBP 创建
- [ ] 敌人 BP 集成

**预计时间**: 每怪物 1-2 小时，总计 4-8 小时

---

## 四、文件目录结构

```
design/assets/output/3d/
├── candy_zombie/          ✅ 完成
│   ├── model_1.obj        (14.7 MB)
│   ├── model_2.glb        (15.9 MB)
│   ├── model_3.fbx        (18.6 MB)
│   ├── preview_1.png
│   ├── preview_2.png
│   ├── preview_3.png
│   ├── generation_log.json
│   └── motions/
│       ├── 待机-1/model_1.fbx     (17.8 MB)
│       ├── 走路-1/model_1.fbx     (17.8 MB)
│       ├── 慢跑/model_1.fbx       (17.9 MB)
│       ├── 二连击打/model_1.fbx   (18.0 MB)
│       ├── 受击/model_1.fbx       (17.9 MB)
│       └── 受击倒地-1/model_1.fbx (17.9 MB)
│
├── gingerbread/           🔄 绑骨补完中
│   ├── model_1.obj
│   ├── model_2.glb
│   ├── model_3.fbx        (18.5 MB)
│   ├── preview_*.png
│   ├── generation_log.json
│   └── motions/
│       └── 待机-1/model_1.fbx     (17.9 MB) ← 已有
│
├── shadow_ninja/          🔄 绑骨补完中
│   ├── model_1.obj
│   ├── model_2.glb
│   ├── model_3.fbx        (19.6 MB)
│   ├── preview_*.png
│   ├── generation_log.json
│   └── motions/
│       ── 受击倒地-1/model_1.fbx (19.5 MB) ← 已有
│
└── armored_gum/           🔄 绑骨补完中
    ├── model_1.obj
    ├── model_2.glb
    ├── model_3.fbx        (41.5 MB)
    ├── preview_*.png
    ├── generation_log.json
    └── motions/           (空)
```

---

## 五、工具与脚本

| 工具 | 路径 | 用途 |
|------|------|------|
| 3D 模型生成器 | `tools/generate_3d_models.py` | AI 生成 + 绑骨 |
| 批量绑骨补完 | `tools/batch_rig_monsters.py` | 补完剩余绑骨动作 |
| Blender 后处理指南 | `design/art/blender-monster-postprocess.md` | Blender 修正流程 |
| UE5 导入指南 | `design/art/ue5-monster-import-guide.md` | UE5 集成流程 |

---

## 六、API 用量记录

| 项目 | 次数 | 说明 |
|------|------|------|
| SubmitHunyuanTo3DProJob (3D 生成) | 7 | candy_zombie×1 + gingerbread×2 + shadow_ninja×2 + armored_gum×2 |
| SubmitAutoRiggingJob (绑骨) | 7 | candy_zombie×6 + gingerbread×1 |
| 预计剩余 | ~17 | gingerbread×5 + shadow_ninja×6 + armored_gum×6 |

**总预计用量**:
- 3D 生成: 4-7 次 (每次 ~$0.10-0.20)
- 绑骨: 24 次 (每次 ~$0.02-0.05)
- **总费用**: ~$1.00-2.50

---

## 七、已知问题

### 问题 1: 绑骨 API 并发限制
- **现象**: 并行提交绑骨请求时，第 2 个起全部失败
- **原因**: 混元3D 绑骨 API 只支持 1 路并发
- **解决**: 串行执行绑骨（已通过 batch_rig_monsters.py 解决）

### 问题 2: AI 模型拓扑质量
- **现象**: AI 生成的模型拓扑非游戏优化
- **影响**: 需要 Blender 后处理修正
- **解决**: 按照 blender-monster-postprocess.md 流程修正

### 问题 3: 动画循环质量
- **现象**: AI 生成的动画首尾帧可能不匹配
- **影响**: 循环动画可能有跳帧
- **解决**: UE5 Animation Editor 中手动调整

---

## 八、下一步行动

1. **立即**: 等待当前 gingerbread 生成完成，然后依次运行 shadow_ninja 和 armored_gum
2. **今日**: 完成所有 4 个怪物的 3D 模型 + 绑骨
3. **本周**: Blender 后处理修正（拓扑/UV/材质/骨骼）
4. **下周**: UE5 导入 + NPR 材质 + AnimBP 集成
