# Asset Specs — Character: 慧空 (Blazing Monkfist)

> **Source**: design/narrative/characters/character-blazing-monkfist.md
> **Art Bible**: design/art/art-bible.md
> **Generated**: 2026-06-16
> **Status**: 12 assets specced

---

## ASSET-001 — 慧空 Sprite Sheet: Idle

| Field | Value |
|-------|-------|
| Category | Sprite / 2D Art |
| Dimensions | 256×256px per frame, 8 frames → 2048×256 sprite sheet |
| Format | PNG (transparent background) |
| Naming | char_huikong_idle_001.png ~ _008.png |
| Texture Res | 256px — matches Art Bible §8 Character tier |

**Visual Description:**
Q版三头身小武僧站立待机，8帧呼吸式微动循环。深蓝色交领武僧袍，红色宽腰带蝴蝶结飘带，金色护手拳套抱于身前，脖子大颗木质念珠，红头带飘带自然下垂，短发小发髻。帧间变化：身体微起伏、飘带轻摆、念珠微晃、眼皮眨动。表情坚毅微笑。

**Art Bible Anchors:**
- §3 Character Proportions: 三头身，头:身:腿 = 1:1:1
- §3 Outline: 2-3px 黑色描边
- §3 Expression: 正常表情（微笑）
- §8 Texture: 256px per sprite

**Generation Prompt:**
```
chibi 3-head-tall Chinese martial artist boy, idle standing pose, breathing micro-animation frame, dark blue cross-collar monk robe, wide red sash belt with bow, golden gauntlet fists held together, large wooden prayer beads around neck, red headband with ribbon, short hair with small topknot, determined gentle smile, 2D hand-drawn cel-shaded style, clean black outline, soft gradient coloring, transparent background, game sprite animation frame, consistent character design, high quality
```

---

## ASSET-002 — 慧空 Sprite Sheet: Walk

| Field | Value |
|-------|-------|
| Category | Sprite / 2D Art |
| Dimensions | 256×256px per frame, 8 frames |
| Format | PNG |
| Naming | char_huikong_walk_001.png ~ _008.png |

**Visual Description:**
8帧弹性步伐走路循环。身体前后微倾，念珠和飘带随步伐摆动，双臂自然摆动，脚步有Q版弹跳感。

**Generation Prompt:**
```
chibi Chinese martial artist boy, walking cycle animation frame, elastic bouncy steps, body leaning forward/back, prayer beads and headband ribbon swinging with steps, arms swinging naturally, Q-version exaggerated bounce, dark blue robe, red belt, golden gauntlets, 2D cel-shaded, clean outline, transparent background, sprite frame
```

---

## ASSET-003 — 慧空 Sprite Sheet: Run

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 6 frames |
| Naming | char_huikong_run_001.png ~ _006.png |

**Visual Description:**
6帧Q版夸张跑步，大幅摆臂，身体前倾，飘带和念珠向后飞扬，速度感强。

**Generation Prompt:**
```
chibi Chinese monk boy, running cycle frame, exaggerated arm swing, body leaning forward, headband ribbon and prayer beads flying backward, speed lines, dynamic pose, 2D cel-shaded, transparent background, sprite frame
```

---

## ASSET-004 — 慧空 Sprite Sheet: Light Attack (3段)

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 5帧/段 × 3段 = 15 frames |
| Naming | char_huikong_light1_001~005, light2_001~005, light3_001~006 |

**Visual Description:**
- 轻攻1: 快速右直拳，5帧（蓄1→出2→命中1→收1），白色小型气功弧特效
- 轻攻2: 左勾拳上挑，5帧，蓝色弧形气劲
- 轻攻3: 双拳升龙上勾，6帧，金色龙形气功波+击飞效果

**Art Bible Anchors:**
- §3 Animation: 轻攻×3 每段4-6帧

**Generation Prompt (light1):**
```
chibi monk boy, fast right straight punch attack frame, white small qi arc effect, dynamic punch pose, fist extended forward, body twisted, 2D cel-shaded, transparent background, sprite frame
```

---

## ASSET-005 — 慧空 Sprite Sheet: Heavy Attack

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 8 frames |
| Naming | char_huikong_heavy_001~008 |

**Visual Description:**
8帧重攻：蓄力3帧（双掌后拉，身体压缩0.7x）→ 推出2帧（双掌前推，身体拉伸1.3x）→ 金色圆形冲击波2帧 → 收招1帧。

**Generation Prompt:**
```
chibi monk boy, heavy palm strike attack, two-phase animation: charging pose (palms pulled back, body compressed) and releasing pose (palms pushed forward, golden circular qi shockwave), 2D cel-shaded, transparent background, sprite frame
```

---

## ASSET-006 — 慧空 Sprite Sheet: Special (必杀)

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 16 frames |
| Naming | char_huikong_special_001~016 |

**Visual Description:**
16帧必杀·如来神掌：蓄力8帧（全身金光脉动，地面碎石浮起，双手合拢金光溢出）→ 佛掌降4帧（巨型金色佛掌从上方出现）→ 砸地4帧（全屏闪白+冲击波+佛掌砸入地面）。

**Generation Prompt:**
```
chibi monk boy, ultimate move "Buddha Palm", charging phase with full body golden aura pulsing, hands pressed together with golden light overflowing, then giant golden Buddha palm descending from above, ground shattering impact, white flash, 2D cel-shaded, dramatic pose, transparent background, sprite frame
```

---

## ASSET-007 — 慧空 Sprite Sheet: Dodge

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 5 frames |
| Naming | char_huikong_dodge_001~005 |

**Visual Description:**
5帧缩身翻滚，金色半透明残影留在原地。

**Generation Prompt:**
```
chibi monk boy, dodge roll animation frame, golden translucent afterimage left behind, compact rolling pose, 2D cel-shaded, transparent background, sprite frame
```

---

## ASSET-008 — 慧空 Sprite Sheet: Hit

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 4 frames |
| Naming | char_huikong_hit_001~004 |

**Visual Description:**
4帧受击：挤压变形0.6x→弹回1.1x→1.0x。表情变痛苦/愤怒，白色星星粒子。

**Generation Prompt:**
```
chibi monk boy, hit reaction frame, body squashed and stretched (impact deformation), pained/angry expression, white star particles, 2D cel-shaded, transparent background, sprite frame
```

---

## ASSET-009 — 慧空 Sprite Sheet: Jump

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 4 frames |
| Naming | char_huikong_jump_001~004 |

**Visual Description:**
4帧跳跃：起跳→滞空→下落→着地。

**Generation Prompt:**
```
chibi monk boy, jump animation frame, takeoff/airborne/landing/grounded poses, 2D cel-shaded, transparent background, sprite frame
```

---

## ASSET-010 — 慧空 Portrait / Avatar

| Field | Value |
|-------|-------|
| Category | UI |
| Dimensions | 96×96px |
| Format | PNG |
| Naming | ui_avatar_huikong_96.png |

**Visual Description:**
慧空正面头像特写，用于HUD左上角。大眼坚毅表情，红头带+念珠清晰可见，圆形裁切。

---

## ASSET-011 — 慧空 Skill Icon: 气功波

| Field | Value |
|-------|-------|
| Category | UI |
| Dimensions | 64×64px |
| Format | PNG |
| Naming | ui_skill_huikong_qi_64.png |

**Visual Description:**
金色拳头+气功波纹图标，圆角方形背景，用于技能栏。

---

## ASSET-012 — 慧空 Skill Icon: 如来神掌

| Field | Value |
|-------|-------|
| Category | UI |
| Dimensions | 64×64px |
| Format | PNG |
| Naming | ui_skill_huikong_buddha_64.png |

**Visual Description:**
金色佛掌图标，圆角方形背景，用于必杀技能栏。
