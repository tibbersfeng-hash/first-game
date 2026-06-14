# Game Concept: 格斗萌主

*Created: 2026-06-15*
*Status: Draft*

---

## Elevator Pitch

> Q版2D横版格斗闯关游戏，玩家操控萌系角色在副本中搓招连击，通过职业成长和装备搭配从萌新冒险者成长为传说中的格斗大师，还能随时和好友来一场1v1切磋。

---

## Core Identity

| Aspect | Detail |
| ---- | ---- |
| **Genre** | 2D横版动作 + ARPG（类DNF） |
| **Platform** | PC（首选），后期可扩展移动端 |
| **Target Audience** | 喜欢动作游戏但嫌DNF太硬核的轻中度玩家 |
| **Player Count** | 单人PVE为主 + 轻量1v1 PVP |
| **Session Length** | 30-60分钟（单副本15-30分钟） |
| **Monetization** | 未定 |
| **Estimated Scope** | 大型（12-18个月 solo，6-9个月小团队） |
| **Comparable Titles** | 地下城与勇士（DNF）、地下城堡、崩坏3 |

---

## Core Fantasy

成为一名Q版格斗大师——小身体蕴含大力量，指尖搓出华丽连招，在地下城中从无名小卒一路打到传奇段位。每次副本都是一场爽快的微型演出，每次切磋都是操作与build的交锋。

---

## Unique Hook

像 DNF 的职业+装备+连招体系，AND ALSO Q版反差萌让打击感更炸——小拳头打出地震级伤害，三头身角色释放必杀时的视觉冲击让人上头。同时简化搓招门槛，3-4键组合就能打出爽快连击，不像DNF那样需要肌肉记忆级别的操作。

---

## Player Experience Analysis (MDA Framework)

### Target Aesthetics (What the player FEELS)

| Aesthetic | Priority | How We Deliver It |
| ---- | ---- | ---- |
| **Sensation** | 1 (核心) | 夸张打击感、屏幕震动、伤害数字弹射、Q版表情变化、技能特效 |
| **Challenge** | 2 (核心) | 连招精通、副本评分、Boss机制、PVP段位 |
| **Fantasy** | 3 (核心) | Q版格斗大师的角色扮演、职业身份认同 |
| **Expression** | 4 (支撑) | 装备build搭配、技能树分支、外观自定义 |
| **Fellowship** | 5 (支撑) | 好友1v1切磋、排行竞争 |
| **Discovery** | 6 (次要) | 新副本探索、隐藏Boss、装备词缀组合 |
| **Narrative** | 7 (次要) | 轻量剧情，服务于世界观氛围 |
| **Submission** | N/A | 不追求放松体验 |

### Key Dynamics (Emergent player behaviors)

- 玩家会反复练习连招组合以追求更高评分和更爽的打击体验
- 玩家会在PVE和PVP之间切换：PVE练build，PVP验操作
- 玩家会尝试不同职业体验截然不同的战斗风格
- 玩家会在装备词缀组合中寻找冷门强力build

### Core Mechanics (Systems we build)

1. **连招战斗系统** — 简化搓招（3-4键组合），命中反馈层（停顿+震动+数字+特效），评分系统
2. **副本闯关系统** — 房间制推进，Boss机制，掉落与评分
3. **职业与成长系统** — 多职业，技能树分支，装备与词缀
4. **轻量PVP系统** — 1v1竞技场，段位排名，好友切磋
5. **城镇与社交系统** — 副本间休整地，排行榜，好友列表

---

## Player Motivation Profile

### Primary Psychological Needs Served

| Need | How This Game Satisfies It | Strength |
| ---- | ---- | ---- |
| **Competence** | 连招精通、副本S评、PVP段位，清晰的技能成长曲线 | Core |
| **Autonomy** | 职业选择、技能树分支、装备build自由搭配 | Core |
| **Relatedness** | 好友切磋、排行竞争、分享build | Supporting |

### Player Type Appeal (Bartle Taxonomy)

- [x] **Achievers** — 副本全S评、装备毕业、职业满级
- [ ] **Explorers** — 次要：新副本、隐藏Boss
- [x] **Socializers** — 好友切磋、分享build、排行
- [x] **Killers/Competitors** — PVP段位、1v1竞技

### Flow State Design

- **Onboarding curve**: 第一个教程副本教普攻+1个技能，3分钟内打出第一个连招
- **Difficulty scaling**: 副本分普通/困难/地狱，Boss机制逐层加复杂度
- **Feedback clarity**: 连招数实时显示、评分即时反馈、伤害数字颜色分级
- **Recovery from failure**: 复活币机制，失败后3秒重进，不惩罚探索

---

## Core Loop

### Moment-to-Moment (30 seconds)
按键输入连招 → 角色释放技能/普攻 → 命中判定 → 打击反馈（停顿+震动+数字+Q版表情） → 连招计数器+1 → 评估下一步输入

### Short-Term (5-15 minutes)
进入副本房间 → 清怪（追求S评连招） → 掉落拾取 → 进入下一房间 → 遇到精英/Boss → 机制应对 → 房间结算评分

### Session-Level (30-120 minutes)
城镇整备（装备/技能/商店） → 选择副本 → 完整通关 → 结算奖励 → 回城镇 → 选择：再来一次/换副本/PVP切磋/下线

### Long-Term Progression
- **力量成长**: 装备强化、属性提升、新技能解锁
- **知识成长**: 掌握职业连招套路、Boss机制记忆、PVP对策
- **选择成长**: 技能树分支选择、装备词缀build搭配
- **社交成长**: PVP段位提升、排行榜排名

### Retention Hooks
- **Curiosity**: 下一个副本的Boss有什么机制？隐藏房间在哪？
- **Investment**: 装备强化投入、职业等级、PVP段位
- **Social**: 好友在等你切磋、排行排名可追赶
- **Mastery**: S评分差一点、连招可以更流畅、PVP可以更强

---

## Game Pillars

### Pillar 1: 爽快打击 + Q版反差
小身体打出大冲击，每次连招都是微型演出，打击感是命根子，Q版反差是灵魂。

*Design test*: 如果要选"加一个新职业"还是"优化打击感反馈"，选打击感。

### Pillar 2: 职业深度与成长
多职业多build，装备搭配驱动长期追求，每个职业玩法截然不同。

*Design test*: 如果要选"加新副本"还是"加新技能树分支"，选技能树。

### Pillar 3: 轻社交竞争
好友切磋、排行竞争，但不强制组队，社交是调味料不是主菜。

*Design test*: 如果要选"做多人团本"还是"做1v1竞技场"，选1v1。

### Anti-Pillars (What This Game Is NOT)

- **NOT 重度社交**: 不做大型团本、公会战、强制组队。这会妥协"爽快打击"支柱——团本中个人操作感被稀释。
- **NOT 硬核操作门槛**: 连招简化到3-4键组合，新手也能爽。追求DNF级别操作深度会吓走Q版受众。
- **NOT 开放世界**: 副本制闯关，不做大地图探索。开放世界会稀释核心战斗密度，范围失控。

---

## Inspiration and References

| Reference | What We Take From It | What We Do Differently | Why It Matters |
| ---- | ---- | ---- | ---- |
| DNF (地下城与勇士) | 连招系统、职业体系、装备成长、副本制 | 简化搓招门槛、Q版美术、轻量PVP | 验证了2D横版格斗+装备成长的模型有极长生命周期 |
| 崩坏3 | 打击感设计、技能特效、Q版3D表现 | 改为2D横版、加入PVP | 验证了Q版角色可以承载极强打击感 |
| 地下城堡 | Roguelike副本结构、装备词缀 | 更侧重动作操作而非数值策略 | 验证了Q版+地牢闯关的受众基础 |

**Non-game inspirations**: 
- 龙珠（小身体大爆发的反差感）
- 一拳超人（一击制霸的爽感节奏）

---

## Target Player Profile

| Attribute | Detail |
| ---- | ---- |
| **Age range** | 18-35 |
| **Gaming experience** | 中度 — 玩过动作游戏但不是硬核玩家 |
| **Time availability** | 工作日30-60分钟，周末更长 |
| **Platform preference** | PC |
| **Current games they play** | DNF（怀旧）、崩坏3、原神 |
| **What they're looking for** | DNF的爽感但不需要10年肌肉记忆，Q版更亲切 |
| **What would turn them away** | 操作太硬核、必须组队才能玩、氪金才能变强 |

---

## Technical Considerations

| Consideration | Assessment |
| ---- | ---- |
| **Recommended Engine** | Godot 4 — 2D横版场景管理优秀、GDScript开发快、免费开源适合独立开发 |
| **Key Technical Challenges** | 连招手感调优（帧数据、输入缓冲、取消窗口）、打击反馈系统（停顿/震动精确控制）、PVP网络同步 |
| **Art Style** | Q版2D像素/手绘风（三头身比例） |
| **Art Pipeline Complexity** | 中等 — Q版2D精灵帧动画，职业数量决定工作量 |
| **Audio Needs** | 中高 — 打击音效是核心体验，需要丰富的命中/技能音效层 |
| **Networking** | PVP用P2P或轻量中继服务器，PVE纯本地 |
| **Content Volume** | 首版3个职业、10个副本（每副本5-8房间）、50+装备、30+技能 |
| **Procedural Systems** | 装备词缀随机组合，副本房间顺序可随机 |

---

## Risks and Open Questions

### Design Risks
- 连招简化后深度不足，老玩家觉得"太简单"不够爽
- PVP平衡难度大，不同职业和build之间的公平性难以保证
- 副本重复感——房间制闯关如果缺乏变化容易腻

### Technical Risks
- 打击手感调优极其耗时，帧数据和取消窗口的精确值需要大量迭代
- PVP网络延迟对格斗游戏是致命的，同步方案需要早期验证
- Q版2D精灵帧动画的美术产出速度可能成为瓶颈

### Market Risks
- DNF仍然是巨兽，同品类竞争激烈
- Q版可能被误判为"儿童向"，需要营销上明确动作硬核定位

### Scope Risks
- 多职业设计范围容易膨胀——每个职业都意味着一套完整动画+技能+平衡
- 装备系统如果做词缀随机，内容量级指数增长

### Open Questions
- 连招系统到底简化到什么程度？需要原型验证 — 计划在 `/prototype combat` 中解决
- PVP用帧同步还是状态同步？需要技术调研 — 计划在架构阶段决定
- Q版2D精灵帧 vs 骨骼动画？影响美术管线 — 计划在 `/setup-engine` 后决定

---

## MVP Definition

**Core hypothesis**: 玩家觉得Q版角色的连招战斗本身足够爽，愿意为了更高评分反复刷副本。

**Required for MVP**:
1. 1个职业的完整连招战斗系统（普攻+3个技能+必杀）
2. 1个3房间副本（含1个Boss）
3. 评分系统（连招数+通关时间）
4. 基础打击反馈（命中停顿+屏幕震动+伤害数字）

**Explicitly NOT in MVP**:
- 多职业（只做1个验证核心）
- 装备系统
- PVP
- 城镇/商店

### Scope Tiers (if budget/time shrinks)

| Tier | Content | Features | Timeline |
| ---- | ---- | ---- | ---- |
| **MVP** | 1职业1副本 | 连招战斗+评分+打击反馈 | 4-6周 |
| **Vertical Slice** | 2职业3副本 | MVP+装备系统+城镇 | 3-4月 |
| **Alpha** | 3职业10副本 | 全功能粗版 | 6-9月 |
| **Full Vision** | 5+职业20+副本 | 全功能打磨+PVP | 12-18月 |

---

## Visual Identity Anchor

**方向**: 糖果炸裂风 — Q版三头身角色在暗色地牢中释放糖果色爆裂技能

**视觉规则**: 一切必须"软萌外壳+硬核内芯"——角色和UI是圆润滑稽的，但技能特效和打击反馈是尖锐爆裂的

**支撑原则**:
1. **软硬反差** — 角色圆润可爱，特效锋利炸裂（设计测试：新特效如果只是"好看"但不"炸"，则不符合）
2. **暗底亮效** — 地牢暗色调，技能和伤害数字是高饱和亮色（设计测试：如果关掉特效画面变暗沉，则正确）
3. **夸张变形** — 命中时角色瞬间挤压拉伸，Q版变形是打击感的关键（设计测试：如果打击变形看起来"只是变大"而非"冲击变形"，需重做）

---

## Next Steps

- [ ] 运行 `/setup-engine` 配置引擎（推荐 Godot 4）
- [ ] 运行 `/art-bible` 创建美术圣经
- [ ] 运行 `/prototype combat` 验证连招战斗核心手感
- [ ] 运行 `/map-systems` 拆解系统依赖
- [ ] 运行 `/design-system [system]` 为每个系统写 GDD
