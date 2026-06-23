# Visual Entity & Screen Inventory

> Generated: 2026-06-16
> Sources: game-concept.md, combat-system.md, combo-system.md, hit-detection-feedback.md, art-bible.md, character-visual-spec-final.md, character-blazing-monkfist.md, character-sugar-hexblade.md, character-oni-crushkin.md

## Characters / Playable

| # | Name | Type | Description | Source | Status |
|---|------|------|-------------|--------|--------|
| 1 | 慧空 (Blazing Monkfist) | Character | 气功小武僧，蓝色武僧袍，金色护手，念珠，红头带 | character-blazing-monkfist.md | Needed |
| 2 | 糖糖 (Sugar Hexblade) | Character | 糖果魔导师，粉色襦裙，薄荷绿披帛，葫芦法杖 | character-sugar-hexblade.md | Needed |
| 3 | 小鬼丸 (Oni Crushkin) | Character | 萌鬼狂战士，紫色皮肤，朱红肚兜，狼牙棒 | character-oni-crushkin.md | Needed |

## Enemies / Creatures

| # | Name | Type | Description | Source | Status |
|---|------|------|-------------|--------|--------|
| 4 | 哥布林小妖 | Enemy | 哥布林洞穴普通怪，绿色皮肤，红色肚兜，木棍 | character_visual.gd | Needed |
| 5 | 幽灵 | Enemy | 幽灵古堡敌人，半透明，漂浮 | dungeon-room.md (implied) | Needed |
| 6 | 熔岩魔 | Enemy | 熔岩地狱敌人，火属性 | dungeon-flow.md (implied) | Needed |
| 7 | 冰晶守卫 | Enemy | 冰晶圣殿敌人，冰属性 | dungeon-flow.md (implied) | Needed |

## Bosses

| # | Name | Type | Description | Source | Status |
|---|------|------|-------------|--------|--------|
| 8 | 哥布林王 | Boss | 哥布林洞穴Boss | dungeon-flow.md (implied) | Needed |
| 9 | 古堡幽灵领主 | Boss | 幽灵古堡Boss | dungeon-flow.md (implied) | Needed |
| 10 | 熔炎魔王 | Boss | 熔岩地狱Boss | dungeon-flow.md (implied) | Needed |
| 11 | 冰晶圣兽 | Boss | 冰晶圣殿Boss | dungeon-flow.md (implied) | Needed |

## VFX / Particles

| # | Name | Type | Description | Source | Status |
|---|------|------|-------------|--------|--------|
| 12 | 轻攻命中 | VFX | 小型圆弧斩击线+3-5粒子，白色 | combat-system.md | Needed |
| 13 | 重攻命中 | VFX | 大型弧形斩击+10-15粒子+闪光，金色 | combat-system.md | Needed |
| 14 | 必杀命中 | VFX | 全屏闪白+冲击波环+30+粒子+速度线 | combat-system.md | Needed |
| 15 | 暴击 | VFX | 额外星形爆炸+屏幕金色叠层 | combat-system.md | Needed |
| 16 | 闪避残影 | VFX | 半透明残影+拖影 | combat-system.md | Needed |
| 17 | 能量充满 | VFX | 角色边缘金色发光脉冲 | combat-system.md | Needed |
| 18 | 击倒灰尘 | VFX | 角色倒地+灰尘爆发 | combat-system.md | Needed |
| 19 | 慧空·气功波 | VFX | 金色龙形气功波 | character-blazing-monkfist.md | Needed |
| 20 | 慧空·如来神掌 | VFX | 巨型金色佛掌从天而降 | character-blazing-monkfist.md | Needed |
| 21 | 糖糖·糖果弹 | VFX | 粉色弹丸+尾迹 | character-sugar-hexblade.md | Needed |
| 22 | 糖糖·地雷引爆 | VFX | 糖果爆炸彩虹扩散 | character-sugar-hexblade.md | Needed |
| 23 | 糖糖·糖果风暴 | VFX | 全屏糖果弹幕雨+彩虹光束 | character-sugar-hexblade.md | Needed |
| 24 | 小鬼丸·震荡波 | VFX | 暗紫色地面冲击波 | character-oni-crushkin.md | Needed |
| 25 | 小鬼丸·鬼王降临 | VFX | 连砸7次+地裂+终击全暗闪白 | character-oni-crushkin.md | Needed |

## UI Screens

| # | Screen Name | Description | Source | Status |
|---|-------------|-------------|--------|--------|
| 26 | HUD 战斗界面 | HP条+能量条+连击数+技能栏 | hud.md | Needed |
| 27 | 副本结算 | 评分+掉落展示 | dungeon-flow.md | Needed |
| 28 | 城镇主界面 | 副本选择+商店+整备 | game-concept.md | Needed |
| 29 | 角色选择 | 3职业选择界面 | game-concept.md | Needed |
| 30 | PVP 匹配 | 1v1 竞技场匹配界面 | game-concept.md | Needed |

## HUD Elements

| # | Element | Description | Source | Status |
|---|---------|-------------|--------|--------|
| 31 | HP 条 | 圆角矩形，绿→黄→红渐变 | art-bible.md | Needed |
| 32 | 能量条 | HP条下方 | combat-system.md | Needed |
| 33 | 连击计数器 | COMBO x N，大字体脉冲 | art-bible.md | Needed |
| 34 | 技能栏图标 | 4格 J/K/L/空格 | art-bible.md | Needed |
| 35 | 副本进度 | 房间 x/N | art-bible.md | Needed |
| 36 | 角色头像 | 48x48 圆形 | art-bible.md | Needed |
| 37 | 伤害数字 | 高饱和+3px描边 | art-bible.md | Needed |

## Environment / Tiles

| # | Name | Type | Description | Source | Status |
|---|------|------|-------------|--------|--------|
| 38 | 哥布林洞穴 tileset | Environment | 棕绿+土黄，火把橙氛围 | art-bible.md | Needed |
| 39 | 幽灵古堡 tileset | Environment | 深紫+灰蓝，幽灵绿氛围 | art-bible.md | Needed |
| 40 | 熔岩地狱 tileset | Environment | 暗红+黑，岩浆橙氛围 | art-bible.md | Needed |
| 41 | 冰晶圣殿 tileset | Environment | 深蓝+白，冰蓝氛围 | art-bible.md | Needed |
| 42 | 城镇背景 | Environment | 暖棕+柔黄，安全放松 | art-bible.md | Needed |

## Audio

| # | Name | Type | Description | Source | Status |
|---|------|------|-------------|--------|--------|
| 43 | 轻攻命中音效 | SFX | 清脆短促 | combat-system.md | Needed |
| 44 | 重攻命中音效 | SFX | 沉重打击感 | combat-system.md | Needed |
| 45 | 必杀释放音效 | SFX | 蓄力音→爆发音 | combat-system.md | Needed |
| 46 | 闪避音效 | SFX | 风声 | combat-system.md | Needed |
| 47 | 击倒音效 | SFX | 重响 | combat-system.md | Needed |
| 48 | 背景音乐-副本 | Music | 紧张压迫感 | game-concept.md | Needed |
| 49 | 背景音乐-城镇 | Music | 温暖放松 | game-concept.md | Needed |
| 50 | 背景音乐-Boss战 | Music | 史诗危险感 | game-concept.md | Needed |
