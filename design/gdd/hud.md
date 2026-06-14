# HUD System

> **Status**: Draft
> **Author**: ui-programmer
> **Last Updated**: 2026-06-15
> **Last Verified**: —
> **Implements Pillar**: 爽快打击+Q版反差 (HUD是战斗信息的即时反馈窗口)

## Summary

HUD系统管理战斗界面上的所有实时信息显示——HP条、能量条、连击计数器、技能栏、波次进度、Boss血条和伤害数字。它遵循"信息即时可见但不遮挡战斗"原则，所有元素半透明+圆角+图标优先，与Q版美术风格一致。

> **Quick reference** — Layer: `Presentation` · Priority: `MVP` · Key deps: `Combo System, Combat System`

## Overview

HUD 是玩家与战斗系统之间的信息桥梁。它不产生任何游戏逻辑，只消费其他系统的数据并可视化。核心原则：战斗区域永远优先——HUD 不能遮挡角色和敌人，信息必须在0.1秒内可读，动画反馈必须增强而非干扰战斗节奏。

## Player Fantasy

看一眼HUD就知道自己还剩多少血、能不能放必杀、连了多少击。连击数大到闪金光时截图的冲动就是HUD设计的成功标志。伤害数字弹出来就是多巴胺——白色还好，金色不错，红色必杀太爽了。

## Detailed Design

### Core Rules

1. **半透明底色**: 所有HUD元素背景alpha 0.6-0.8，不遮挡战斗区
2. **圆角矩形**: 所有条/框 radius ≥ 8px，与Q版美术风格一致
3. **图标优先**: 技能用图标+颜色编码，减少文字依赖
4. **信息层级**: 关键信息(HP/combo)大而明显，次要信息(波次/计时)小而安静
5. **动画不干扰**: HUD动画≤0.3秒，不阻挡战斗阅读
6. **可配置**: HUD布局和透明度可通过设置调整

### HUD Layout (1280×720)

```
┌──────────────────────────────────────────────────────┐
│ [头像] [████████HP████████] [███MP███]   Room 3/8  │ ← 顶部栏
│                                                      │
│                                                      │
│                    COMBO x12                         │ ← 连击(中上)
│                                                      │
│                    ★ 基础连 ★                        │ ← 连招名(连击下)
│                                                      │
│                                                      │
│                                                      │
│                                          [J][K][L][→]│ ← 技能栏(右下)
└──────────────────────────────────────────────────────┘
```

### HUD Elements

| Element | Position | Size | Priority | Update Freq |
|---------|----------|------|----------|-------------|
| 角色头像 | 左上 | 48×48px | High | 静态 |
| HP条 | 左上(头像右) | 200×24px | Critical | 每帧 |
| 能量条 | HP条下方 | 200×16px | High | 每帧 |
| 连击数 | 中上 | 动态缩放 | Critical | 每次命中 |
| 连招名 | 连击数下方 | 小字 | Medium | 连招完成时 |
| 技能栏 | 右下 | 4×48px图标 | High | 状态变化时 |
| 波次进度 | 右上 | 小字 | Low | 波次变化时 |
| 复活币 | HP条旁 | 小图标 | Medium | 消耗时 |
| Boss HP | 底部全宽 | 大条 | High(仅Boss) | 每帧 |
| 伤害数字 | 命中位置 | 动态 | Critical | 每次命中 |

### HP Bar Design

| State | Fill Color | Animation |
|-------|-----------|-----------|
| HP > 60% | #00CC44 (绿) | 无 |
| HP 30-60% | #FFAA00 (黄) | 微弱脉冲 |
| HP < 30% | #FF3333 (红) | 明显脉冲+屏幕边缘红晕 |
| HP变化 | 渐变过渡 | 0.2s平滑过渡，先快后慢 |
| 受伤 | 红色闪烁 | 0.1s闪红→恢复 |

### Energy Bar Design

| State | Fill Color | Animation |
|-------|-----------|-----------|
| energy < 30 | #5566AA (暗蓝) | 无 |
| energy ≥ 30 (必杀可用) | #6699FF (亮蓝) | 技能栏必杀图标闪烁 |
| energy = 100 (满) | #00F5D4 (能量青) | 角色边缘发光脉冲 |
| energy变化 | 渐变过渡 | 0.15s平滑 |

### Combo Counter Design

| Combo Count | Font Size | Color | Animation |
|------------|----------|-------|-----------|
| 1-4 | 32px | #FFFFFF (白) | 命中时放大120%→恢复 |
| 5-9 | 36px | #FFFFAA (浅金) | 命中时放大130%+轻微震动 |
| 10-14 | 42px | #FFD700 (金) | 放大140%+粒子+屏幕微色差 |
| 15-19 | 48px | #FF8800 (橙) | 放大150%+强粒子+色差 |
| 20+ | 56px | #FF4444 (红金) | 放大160%+全屏脉冲+金光 |

**Combo消失动画**: 红色闪烁→缩小→淡出(0.5秒)

### Skill Bar Design

4个技能槽位，从左到右：

| Slot | Key | Icon | Color | Cooldown Visual |
|------|-----|------|-------|----------------|
| 1 | J | 剑图标 | 白色边框 | 无CD(轻攻) |
| 2 | K | 锤图标 | 黄色边框 | 能量<5时暗灰 |
| 3 | L | 星图标 | 蓝色边框 | 能量<30时暗灰+闪烁提示 |
| 4 | Shift | 风图标 | 绿色边框 | 能量<8时暗灰 |

**CD显示**: 扇形遮罩从上顺时针消除，剩余CD秒数显示在图标中央

### Boss HP Bar

- 位置: 屏幕底部全宽(左右各留20px边距)
- 高度: 20px
- 分段: 根据Boss阶段数量分段(2段=2阶段, 3段=3阶段)
- 阶段切换: 当前阶段段颜色实心，后续段半透明
- Boss名: HP条上方居中显示

### Damage Numbers (详见 Hit Detection GDD)

HUD系统负责伤害数字的生成和显示，参数由Hit Detection GDD定义：

| Type | Size | Color | Outline | Extra |
|------|------|-------|---------|-------|
| Light | 32px | #FFFFFF | 3px黑 | 无 |
| Heavy | 42px | #FFD700 | 3px黑 | 无 |
| Special | 48px | #FF4444 | 3px黑 | 无 |
| Crit | 48px | #FFD700 | 4px黑 | 后缀"!" |

**动画**: 弹出→上升→淡出(800ms)，方向随机偏移±30px

### States and Transitions

| State | Condition | Visible Elements | Notes |
|-------|-----------|-----------------|-------|
| Exploration | 城镇/走廊 | HP+能量+技能栏 | 无连击/Boss条 |
| Combat | 战斗中 | 全部 | 连击/Boss条出现 |
| Boss | Boss战 | 全部 | Boss HP条+Boss名 |
| Paused | 暂停 | 全部冻结 | 半透明遮罩 |
| Cutscene | 过场 | 无 | 隐藏所有HUD |
| Death | 玩家死亡 | HP条闪红+复活选择 | 其他元素暗化 |

### Interactions with Other Systems

| System | Data Consumed | Update Trigger |
|--------|--------------|---------------|
| Player Controller | HP, position, state | 每帧 |
| Combat System | energy, attack state | 每帧/事件 |
| Combo System | combo_count, chain_name | 每次命中 |
| Hit Detection | damage, type, is_crit, position | 每次命中 |
| Dungeon Room | wave_progress, room_rating | 波次/房间变化 |
| Dungeon Flow | revive_coins, room_index, boss_hp | 事件 |
| Input System | 技能按键状态 | 每帧 |

## Formulas

### HP Bar Width

```
bar_width = max(bar_max_width * (current_hp / max_hp), min_visible_width)
```

| Variable | Type | Range | Source | Description |
|----------|------|-------|--------|-------------|
| bar_max_width | int | 200 | config | HP条最大宽度(px) |
| current_hp | int | 0-max | player | 当前HP |
| max_hp | int | 100-500 | config | 最大HP |
| min_visible_width | int | 4 | config | 最小可见宽度(HP近0时) |

### Combo Font Size

```
base_size = 32
size = base_size + min(combo_count, 20) * 1.2
```

**Expected**: 32px (1 combo) → 56px (20+ combo)

### Damage Number Rise

```
position.y -= rise_speed * delta * (1 - age/lifetime)
alpha = clamp(age / (lifetime * 0.3), 0, 1) when age > lifetime * 0.7
```

## Edge Cases

| Scenario | Expected Behavior | Rationale |
|----------|------------------|-----------|
| 多个伤害数字重叠 | 随机X偏移±30px减少重叠 | 可读性 |
| HP条为0 | 保留4px最小可见宽度 | 玩家知道已死 |
| 能量刚好30按必杀 | 能量条同步扣减，不延迟 | 视觉同步 |
| Boss HP条分段转场 | 旧阶段段变灰，新阶段段亮起 | 清晰阶段感 |
| 20连击+暴击同时 | 连击放大+伤害数字金色叹号 | 双重反馈 |
| HUD动画期间被击 | HUD动画不阻塞，立即更新 | 不影响战斗 |
| 窗口缩放 | HUD按比例缩放 | 适配不同分辨率 |

## Dependencies

| System | Direction | Nature of Dependency |
|--------|-----------|---------------------|
| Combat System | This consumes Combat | energy/attack state |
| Combo System | This consumes Combo | combo_count/chain_name |
| Hit Detection | This consumes Hit Detection | damage/type/crit |
| Player Controller | This consumes Player | HP/state |
| Dungeon Room | This consumes Room | wave/progress |
| Dungeon Flow | This consumes Flow | revive_coins/room_index |

## Tuning Knobs

| Parameter | Current Value | Safe Range | Effect of Increase | Effect of Decrease |
|-----------|--------------|------------|-------------------|-------------------|
| hp_bar_width | 200px | 120-300 | HP条更宽 | HP条更窄 |
| hp_bar_height | 24px | 16-40 | HP条更粗 | HP条更细 |
| energy_bar_height | 16px | 10-24 | 能量条更粗 | 能量条更细 |
| combo_base_size | 32px | 24-40 | 连击字更大 | 连击字更小 |
| combo_scale_rate | 1.2px/hit | 0.5-2.0 | 连击字增长更快 | 增长更慢 |
| dmg_number_lifetime | 800ms | 500-1200 | 数字停留更久 | 数字消失更快 |
| dmg_number_rise | 80px/s | 40-150 | 数字上升更快 | 上升更慢 |
| hud_bg_alpha | 0.7 | 0.4-0.9 | 底色更不透明 | 底色更透明 |
| low_hp_threshold | 30% | 15-40 | 低血量警告更早 | 更晚警告 |

## Visual/Audio Requirements

| Event | Visual Feedback | Audio Feedback | Priority |
|-------|----------------|---------------|----------|
| HP<30% | HP条红脉冲+屏幕边缘红晕 | 低HP警告音(循环) | High |
| 必杀可用 | 能量条亮蓝+技能图标闪烁 | 能量充满提示音 | High |
| 能量充满 | 角色边缘青色发光 | 能量满音效 | Medium |
| 连击10+ | 屏幕轻微色差 | 连击升调音效 | Medium |
| 连击20+ | 全屏脉冲 | 高连击音效 | High |
| 受伤 | HP条红闪(0.1s) | 受伤音效 | High |
| 技能CD中 | 扇形遮罩+秒数 | 无 | Low |

## Game Feel

### Feel Reference
应像《苍翼默示录》的HUD——信息清晰但不喧宾夺主，连击数字是奖励而非负担。伤害数字弹出来就是多巴胺。**不应像**MMO的密集HUD——那在动作游戏中是灾难。

### Feel Acceptance Criteria

- [ ] HP/能量条0.1秒内可读当前值
- [ ] 连击5+时视觉明显升级，10+时测试者注意到
- [ ] 伤害数字颜色编码清晰，白/金/红一眼区分
- [ ] 低血量(30%)视觉警告明显但不焦虑
- [ ] HUD不遮挡战斗核心区域（角色+敌人）
- [ ] Boss HP条清晰显示阶段分段

## UI Requirements

(本GDD即HUD的UI规格，此节整合)

| Element | Spec |
|---------|------|
| HP条 | 200×24px, 左上, 绿→黄→红渐变, 0.2s平滑过渡 |
| 能量条 | 200×16px, HP下方, 暗蓝→亮蓝→青 |
| 连击数 | 中上, 32-56px动态缩放, 白→金→红金 |
| 技能栏 | 右下, 4×48px图标, CD扇形遮罩 |
| Boss HP | 底部全宽, 20px, 阶段分段 |
| 伤害数字 | 命中位置, 32-48px, 800ms弹出上升淡出 |

## Cross-References

| This Document References | Target GDD | Specific Element Referenced | Nature |
|--------------------------|-----------|----------------------------|--------|
| 伤害数字参数 | `design/gdd/hit-detection-feedback.md` | 颜色/大小/动画 | Rule dependency |
| 连击数来源 | `design/gdd/combo-system.md` | combo_count/chain_name | Data dependency |
| 能量值来源 | `design/gdd/combat-system.md` | energy值 | Data dependency |
| HP值来源 | `design/gdd/player-controller.md` | current_hp/max_hp | Data dependency |
| HUD视觉风格 | `design/art/art-bible.md` | UI Art Standards | Rule dependency |

## Acceptance Criteria

- [ ] HP条三色渐变(绿→黄→红)正确显示，0.2s平滑过渡
- [ ] 能量条<30暗蓝/≥30亮蓝/=100青色，必杀可用时技能图标闪烁
- [ ] 连击数1-20+字体大小/颜色/动画按5级递进
- [ ] 伤害数字4色编码(白/金/红/金叹号)正确显示
- [ ] Boss HP条分段显示阶段
- [ ] 技能CD扇形遮罩+秒数
- [ ] HP<30%屏幕边缘红晕+HP条脉冲
- [ ] HUD不遮挡战斗区(角色+敌人在HUD元素之外)
- [ ] Performance: HUD update < 0.5ms/帧
- [ ] No hardcoded values in implementation

## Open Questions

| Question | Owner | Deadline | Resolution |
|----------|-------|----------|-----------|
| 是否提供"隐藏HUD"选项？ | game-designer | GDD审批时 | 倾向提供 |
| 伤害数字是否可调密度？ | game-designer | 设置系统时 | 简化/标准/详细 |
| 小地图？ | game-designer | VS阶段 | MVP不需要 |
